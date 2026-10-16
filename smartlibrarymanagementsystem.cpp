#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <deque>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

// ==========================================
// 1. ANSI COLOR CODES
// ==========================================
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[1;31m";
    const std::string GREEN   = "\033[1;32m";
    const std::string YELLOW  = "\033[1;33m";
    const std::string BLUE    = "\033[1;34m";
    const std::string MAGENTA = "\033[1;35m";
    const std::string CYAN    = "\033[1;36m";
    const std::string BOLD    = "\033[1m";
}

// ==========================================
// 2. ENUMS & CONSTANTS
// ==========================================
enum class SeatStatus { FREE, OCCUPIED, ON_BREAK };
enum class ZoneType { SILENT_ZONE, DISCUSSION_POD, PC_LAB };

std::string zoneToString(ZoneType z) {
    switch (z) {
        case ZoneType::SILENT_ZONE:    return "Silent Zone";
        case ZoneType::DISCUSSION_POD: return "Discussion Pod";
        case ZoneType::PC_LAB:         return "PC Lab";
    }
    return "Standard";
}

// ==========================================
// 3. TRIE DATA STRUCTURE (STUDENT AUTO-COMPLETE)
// ==========================================
class TrieNode {
public:
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    bool isEndOfWord;
    std::vector<std::string> associatedNames;

    TrieNode() : isEndOfWord(false) {}
};

class StudentTrie {
private:
    std::shared_ptr<TrieNode> root;

    void collectAllWords(std::shared_ptr<TrieNode> curr, std::vector<std::string>& results) {
        if (!curr) return;
        if (curr->isEndOfWord) {
            for (const auto& name : curr->associatedNames) {
                results.push_back(name);
            }
        }
        for (auto it = curr->children.begin(); it != curr->children.end(); ++it) {
            collectAllWords(it->second, results);
        }
    }

public:
    StudentTrie() : root(std::make_shared<TrieNode>()) {}

    void insert(const std::string& key, const std::string& fullName) {
        auto curr = root;
        for (char ch : key) {
            char lower = tolower(ch);
            if (curr->children.find(lower) == curr->children.end()) {
                curr->children[lower] = std::make_shared<TrieNode>();
            }
            curr = curr->children[lower];
        }
        curr->isEndOfWord = true;
        curr->associatedNames.push_back(fullName);
    }

    std::vector<std::string> searchPrefix(const std::string& prefix) {
        auto curr = root;
        std::vector<std::string> results;
        for (char ch : prefix) {
            char lower = tolower(ch);
            if (curr->children.find(lower) == curr->children.end()) {
                return results; // No matches found
            }
            curr = curr->children[lower];
        }
        collectAllWords(curr, results);
        return results;
    }
};

// ==========================================
// 4. OOP: USER HIERARCHY
// ==========================================
class User {
protected:
    std::string name;
    std::string mobile;

public:
    User(std::string n, std::string m) : name(std::move(n)), mobile(std::move(m)) {}
    virtual ~User() = default;

    const std::string& getName() const { return name; }
    const std::string& getMobile() const { return mobile; }

    virtual std::string getUserRole() const = 0;
    virtual int getMaxDurationAllowed() const = 0; // In minutes
};

class StudentMember : public User {
private:
    std::string rollNo;

public:
    StudentMember(std::string n, std::string m, std::string roll)
        : User(std::move(n), std::move(m)), rollNo(std::move(roll)) {}

    std::string getUserRole() const override { return "Student"; }
    int getMaxDurationAllowed() const override { return 180; } // 3 Hours
    const std::string& getRollNo() const { return rollNo; }
};

class FacultyMember : public User {
private:
    std::string department;

public:
    FacultyMember(std::string n, std::string m, std::string dept)
        : User(std::move(n), std::move(m)), department(std::move(dept)) {}

    std::string getUserRole() const override { return "Faculty (" + department + ")"; }
    int getMaxDurationAllowed() const override { return 360; } // 6 Hours
};

// ==========================================
// 5. OOP: BOOKING & BREAK MANAGEMENT
// ==========================================
class Booking {
private:
    int seatId;
    std::shared_ptr<User> user;
    int durationMinutes;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::chrono::system_clock::time_point breakEndTime;
    bool onBreak;
    std::string entryTimeFormatted;

public:
    Booking(int sid, std::shared_ptr<User> u, int durationMins, std::string entryTime)
        : seatId(sid), user(std::move(u)), durationMinutes(durationMins), onBreak(false), entryTimeFormatted(std::move(entryTime)) {
        startTime = std::chrono::system_clock::now();
        endTime = startTime + std::chrono::minutes(durationMinutes);
    }

    int getSeatId() const { return seatId; }
    std::shared_ptr<User> getUser() const { return user; }
    int getDuration() const { return durationMinutes; }
    const std::string& getEntryTime() const { return entryTimeFormatted; }
    std::chrono::system_clock::time_point getEndTime() const { return endTime; }
    bool isOnBreak() const { return onBreak; }

    void startBreak(int breakMinutes = 15) {
        onBreak = true;
        breakEndTime = std::chrono::system_clock::now() + std::chrono::minutes(breakMinutes);
    }

    void endBreak() {
        onBreak = false;
    }

    bool isBreakExpired() const {
        return onBreak && (std::chrono::system_clock::now() >= breakEndTime);
    }

    int getBreakRemainingMinutes() const {
        if (!onBreak) return 0;
        auto now = std::chrono::system_clock::now();
        if (now >= breakEndTime) return 0;
        return static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(breakEndTime - now).count());
    }

    bool isExpired() const {
        return std::chrono::system_clock::now() >= endTime;
    }

    int getRemainingMinutes() const {
        auto now = std::chrono::system_clock::now();
        if (now >= endTime) return 0;
        return static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(endTime - now).count());
    }
};

class Seat {
private:
    int seatId;
    ZoneType zone;
    SeatStatus status;
    std::shared_ptr<Booking> currentBooking;

public:
    Seat(int id, ZoneType z) : seatId(id), zone(z), status(SeatStatus::FREE), currentBooking(nullptr) {}

    int getId() const { return seatId; }
    ZoneType getZone() const { return zone; }
    SeatStatus getStatus() const { return status; }
    std::shared_ptr<Booking> getBooking() const { return currentBooking; }

    void assignBooking(std::shared_ptr<Booking> b) {
        currentBooking = std::move(b);
        status = SeatStatus::OCCUPIED;
    }

    void setStatus(SeatStatus s) { status = s; }

    void vacate() {
        currentBooking = nullptr;
        status = SeatStatus::FREE;
    }
};

// ==========================================
// 6. WAITING LIST & EXPIRY MIN-HEAP
// ==========================================
struct WaitingPerson {
    std::shared_ptr<User> user;
    int durationMinutes;
    std::string entryTime;
    ZoneType preferredZone;
};

struct ExpiryComparator {
    bool operator()(const std::shared_ptr<Booking>& a, const std::shared_ptr<Booking>& b) const {
        return a->getEndTime() > b->getEndTime();
    }
};

// ==========================================
// 7. CORE LIBRARY CONTROLLER
// ==========================================
class SmartLibrarySystem {
private:
    static const int NUM_SEATS = 50;
    static const int ROWS = 5;
    static const int COLS = 10;

    // --- DATA STRUCTURES ---
    std::vector<std::shared_ptr<Seat>> seatGrid;                             // 1. Vector: 2D Floor Layout
    std::unordered_map<int, std::shared_ptr<Booking>> activeBookings;        // 2. Hash Map: Fast O(1) Booking lookup
    std::unordered_set<int> freeSeatIds;                                     // 3. Hash Set: O(1) Free seat tracker
    std::queue<WaitingPerson> waitingQueue;                                  // 4. Queue (FIFO): Waiting List
    std::stack<std::string> operationStack;                                  // 5. Stack (LIFO): Recent operations
    std::priority_queue<std::shared_ptr<Booking>, 
                        std::vector<std::shared_ptr<Booking>>, 
                        ExpiryComparator> expiryMinHeap;                     // 5. Min-Heap: Expiration scheduler
    std::deque<std::string> eventAuditLog;                                   // 6. Deque: Recent live audit log
    StudentTrie studentTrie;                                                 // 7. Trie: Prefix auto-complete
    std::map<std::string, int> studyHoursLeaderboard;                        // 8. Map: Student study leaderboard

    std::mutex mtx;
    std::atomic<bool> isRunning;

public:
    SmartLibrarySystem() : isRunning(true) {
        seatGrid.reserve(NUM_SEATS);
        for (int i = 1; i <= NUM_SEATS; ++i) {
            ZoneType zone = (i <= 20) ? ZoneType::SILENT_ZONE : 
                            ((i <= 40) ? ZoneType::DISCUSSION_POD : ZoneType::PC_LAB);
            seatGrid.push_back(std::make_shared<Seat>(i, zone));
            freeSeatIds.insert(i);
        }
        addLog("System initialized: 50 Seats (Silent, Discussion, PC Lab).");
    }

    ~SmartLibrarySystem() { isRunning = false; }

    void addLog(const std::string& msg) {
        if (eventAuditLog.size() >= 6) eventAuditLog.pop_front();
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&now), "%H:%M:%S") << "] " << msg;
        eventAuditLog.push_back(ss.str());
    }

    // ------------------------------------------
    // 1. BOOK SINGLE SEAT
    // ------------------------------------------
    bool bookSeat(int seatId, const std::string& name, const std::string& mobile, int durationMins, const std::string& entryTime, bool isStudent = true) {
        std::lock_guard<std::mutex> lock(mtx);

        if (seatId < 1 || seatId > NUM_SEATS || freeSeatIds.find(seatId) == freeSeatIds.end()) {
            std::cout << Color::RED << "❌ Seat unavailable or invalid!\n" << Color::RESET;
            return false;
        }

        std::shared_ptr<User> user;
        if (isStudent) {
            user = std::make_shared<StudentMember>(name, mobile, "STU-" + std::to_string(seatId + 1000));
        } else {
            user = std::make_shared<FacultyMember>(name, mobile, "Computer Science");
        }

        auto booking = std::make_shared<Booking>(seatId, user, durationMins, entryTime);
        seatGrid[seatId - 1]->assignBooking(booking);
        activeBookings[seatId] = booking;
        freeSeatIds.erase(seatId);
        expiryMinHeap.push(booking);

        // Index in Trie & Update Leaderboard
        studentTrie.insert(name, name + " (Seat " + std::to_string(seatId) + " - " + user->getUserRole() + ")");
        studyHoursLeaderboard[name] += durationMins;

        operationStack.push("Booked Seat " + std::to_string(seatId) + " by " + name);
        addLog("Seat " + std::to_string(seatId) + " reserved by " + name);
        std::cout << Color::GREEN << "✅ Seat " << seatId << " successfully booked for " << name << "!\n" << Color::RESET;
        return true;
    }

    // ------------------------------------------
    // 2. GROUP BOOKING (FIND K ADJACENT SEATS)
    // ------------------------------------------
    void bookGroupSeats(int groupSize, const std::string& leadName, const std::string& mobile, int duration) {
        std::lock_guard<std::mutex> lock(mtx);

        if (groupSize <= 1 || groupSize > COLS) {
            std::cout << Color::RED << "❌ Group size must be between 2 and " << COLS << " seats.\n" << Color::RESET;
            return;
        }

        // Search for consecutive free seats in any row
        int bestStartSeat = -1;
        for (int r = 0; r < ROWS; ++r) {
            int consecutive = 0;
            int startSid = -1;
            for (int c = 0; c < COLS; ++c) {
                int sid = r * COLS + c + 1;
                if (freeSeatIds.find(sid) != freeSeatIds.end()) {
                    if (consecutive == 0) startSid = sid;
                    consecutive++;
                    if (consecutive == groupSize) {
                        bestStartSeat = startSid;
                        break;
                    }
                } else {
                    consecutive = 0;
                }
            }
            if (bestStartSeat != -1) break;
        }

        if (bestStartSeat == -1) {
            std::cout << Color::RED << "❌ No consecutive " << groupSize << " adjacent seats found in the same row!\n" << Color::RESET;
            return;
        }

        std::cout << Color::GREEN << "🎉 Found " << groupSize << " adjacent seats starting from Seat #" << bestStartSeat << "!\n" << Color::RESET;
        for (int i = 0; i < groupSize; ++i) {
            int sid = bestStartSeat + i;
            std::string memberName = leadName + "_Member" + std::to_string(i + 1);
            auto user = std::make_shared<StudentMember>(memberName, mobile, "GRP-" + std::to_string(sid));
            auto booking = std::make_shared<Booking>(sid, user, duration, "Now");

            seatGrid[sid - 1]->assignBooking(booking);
            activeBookings[sid] = booking;
            freeSeatIds.erase(sid);
            expiryMinHeap.push(booking);
            studentTrie.insert(memberName, memberName + " (Seat " + std::to_string(sid) + ")");
            studyHoursLeaderboard[memberName] += duration;
        }

        operationStack.push("Group booked " + std::to_string(groupSize) + " seats by " + leadName);
        addLog("Group reservation: " + std::to_string(groupSize) + " seats (Lead: " + leadName + ")");
    }

    // ------------------------------------------
    // 3. TOGGLE BREAK / HOLD MODE (15 MINS)
    // ------------------------------------------
    void toggleBreak(int seatId) {
        std::lock_guard<std::mutex> lock(mtx);
        if (activeBookings.find(seatId) == activeBookings.end()) {
            std::cout << Color::RED << "❌ Seat is not occupied!\n" << Color::RESET;
            return;
        }

        auto b = activeBookings[seatId];
        if (!b->isOnBreak()) {
            b->startBreak(15);
            seatGrid[seatId - 1]->setStatus(SeatStatus::ON_BREAK);
            operationStack.push("Seat " + std::to_string(seatId) + " put ON BREAK");
            addLog("Seat " + std::to_string(seatId) + " is now ON BREAK (15m hold).");
            std::cout << Color::YELLOW << "⏸ Seat " << seatId << " marked ON BREAK. 15-minute hold started.\n" << Color::RESET;
        } else {
            b->endBreak();
            seatGrid[seatId - 1]->setStatus(SeatStatus::OCCUPIED);
            operationStack.push("Seat " + std::to_string(seatId) + " returned from break");
            addLog("Seat " + std::to_string(seatId) + " occupant returned from break.");
            std::cout << Color::GREEN << "▶ Seat " << seatId << " resumed active status.\n" << Color::RESET;
        }
    }

    // ------------------------------------------
    // 4. FREE SEAT & AUTO-DISPATCH WAITING QUEUE
    // ------------------------------------------
    bool freeSeat(int seatId) {
        std::lock_guard<std::mutex> lock(mtx);

        if (activeBookings.find(seatId) == activeBookings.end()) {
            std::cout << Color::RED << "❌ Seat is not occupied.\n" << Color::RESET;
            return false;
        }

        std::string occupant = activeBookings[seatId]->getUser()->getName();
        seatGrid[seatId - 1]->vacate();
        activeBookings.erase(seatId);
        freeSeatIds.insert(seatId);
        operationStack.push("Released Seat " + std::to_string(seatId) + " (was " + occupant + ")");
        addLog("Seat " + std::to_string(seatId) + " vacated (was " + occupant + ").");

        // Check if someone is in the Waiting Queue
        if (!waitingQueue.empty()) {
            WaitingPerson next = waitingQueue.front();
            waitingQueue.pop();

            auto booking = std::make_shared<Booking>(seatId, next.user, next.durationMinutes, next.entryTime);
            seatGrid[seatId - 1]->assignBooking(booking);
            activeBookings[seatId] = booking;
            freeSeatIds.erase(seatId);
            expiryMinHeap.push(booking);

            addLog("Seat " + std::to_string(seatId) + " AUTO-ASSIGNED to waiting student: " + next.user->getName());
            std::cout << Color::CYAN << "⚡ Auto-Assigned Seat " << seatId << " to waiting student: " << next.user->getName() << "!\n" << Color::RESET;
        }

        return true;
    }

    // ------------------------------------------
    // 5. JOIN WAITING LIST
    // ------------------------------------------
    void joinWaitingQueue(const std::string& name, const std::string& mobile, int duration) {
        std::lock_guard<std::mutex> lock(mtx);
        auto user = std::make_shared<StudentMember>(name, mobile, "WAIT-" + std::to_string(waitingQueue.size() + 1));
        waitingQueue.push({user, duration, "Now", ZoneType::SILENT_ZONE});
        operationStack.push("Added " + name + " to Waiting Queue");
        addLog("Student " + name + " joined the Waiting Queue (Position #" + std::to_string(waitingQueue.size()) + ")");
        std::cout << Color::CYAN << "⏳ " << name << " added to Waiting List! Position: #" << waitingQueue.size() << "\n" << Color::RESET;
    }

    // ------------------------------------------
    // 6. AUTO-EXPIRY WORKER (TIMERS & BREAKS)
    // ------------------------------------------
    void checkAndExpireBookings() {
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::system_clock::now();

        // 1. Check expired breaks
        for (auto it = activeBookings.begin(); it != activeBookings.end(); ++it) {
            auto b = it->second;
            if (b->isBreakExpired()) {
                int sid = b->getSeatId();
                std::cout << Color::RED << "\n⏰ Break expired for Seat " << sid << "! Auto-releasing...\n" << Color::RESET;
                b->endBreak();
                seatGrid[sid - 1]->vacate();
                it = activeBookings.erase(it);
                freeSeatIds.insert(sid);
                addLog("Seat " + std::to_string(sid) + " break expired $\to$ Auto-freed.");
                if (it == activeBookings.end()) break;
            }
        }

        // 2. Check expired full booking sessions via Min-Heap
        while (!expiryMinHeap.empty()) {
            auto top = expiryMinHeap.top();
            if (top->isExpired()) {
                expiryMinHeap.pop();
                int sid = top->getSeatId();
                if (activeBookings.find(sid) != activeBookings.end() && activeBookings[sid] == top) {
                    seatGrid[sid - 1]->vacate();
                    activeBookings.erase(sid);
                    freeSeatIds.insert(sid);
                    addLog("Seat " + std::to_string(sid) + " session expired $\to$ Auto-freed.");
                }
            } else {
                break;
            }
        }
    }

    // ------------------------------------------
    // 7. TRIE PREFIX AUTO-COMPLETE SEARCH
    // ------------------------------------------
    void searchStudentByName(const std::string& prefix) {
        std::lock_guard<std::mutex> lock(mtx);
        auto matches = studentTrie.searchPrefix(prefix);
        std::cout << Color::CYAN << "\n🔍 Search Results for prefix \"" << prefix << "\":" << Color::RESET << "\n";
        if (matches.empty()) {
            std::cout << "  No registered student records found matching this prefix.\n";
        } else {
            for (const auto& match : matches) {
                std::cout << "  👉 " << match << "\n";
            }
        }
    }

    // ------------------------------------------
    // 8. LEADERBOARD DISPLAY
    // ------------------------------------------
    void showLeaderboard() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << Color::MAGENTA << "\n🏆 LIBRARY TOP SCHOLARS (Study Hours Leaderboard)" << Color::RESET << "\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::left << std::setw(6) << "Rank" << std::setw(25) << "Student Name" << "Total Study Time\n";
        std::cout << "--------------------------------------------------------\n";

        // Sort by minutes descending
        std::vector<std::pair<std::string, int>> ranked(studyHoursLeaderboard.begin(), studyHoursLeaderboard.end());
        std::sort(ranked.begin(), ranked.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });

        int rank = 1;
        for (const auto& entry : ranked) {
            std::cout << std::left << std::setw(6) << ("#" + std::to_string(rank++))
                      << std::setw(25) << entry.first
                      << entry.second << " mins\n";
            if (rank > 5) break; // Top 5
        }
        std::cout << "--------------------------------------------------------\n";
    }

    // ------------------------------------------
    // 9. STACK: RECENT OPERATIONS (LIFO)
    // ------------------------------------------
    void showRecentOperations() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << Color::MAGENTA << "\n📚 RECENT LIBRARY OPERATIONS (Stack - LIFO)" << Color::RESET << "\n";
        std::cout << "-----------------------------------------------\n";

        if (operationStack.empty()) {
            std::cout << "  No operations recorded yet.\n";
        } else {
            std::stack<std::string> temp = operationStack;
            int count = 1;
            while (!temp.empty() && count <= 10) {
                std::cout << "  " << count++ << ". " << temp.top() << "\n";
                temp.pop();
            }
        }
        std::cout << "-----------------------------------------------\n";
        std::cout << "Stack follows LIFO: Last operation is displayed first.\n";
    }

    void resetAll() {
        std::lock_guard<std::mutex> lock(mtx);
        for (size_t i = 0; i < seatGrid.size(); ++i) seatGrid[i]->vacate();
        activeBookings.clear();
        freeSeatIds.clear();
        for (int i = 1; i <= NUM_SEATS; ++i) freeSeatIds.insert(i);
        while (!waitingQueue.empty()) waitingQueue.pop();
        while (!expiryMinHeap.empty()) expiryMinHeap.pop();
        operationStack.push("Reset all library seats");
        addLog("ALL SEATS RESET.");
        std::cout << Color::GREEN << "♻ All seats reset successfully!\n" << Color::RESET;
    }

    // ------------------------------------------
    // 9. RENDER LIVE COLOR DASHBOARD
    // ------------------------------------------
    void renderDashboard() {
        std::lock_guard<std::mutex> lock(mtx);

        std::cout << "\n" << Color::BOLD << Color::BLUE 
                  << "=========================================================================================\n"
                  << "                   📚 ADVANCED SMART LIBRARY SEAT MANAGEMENT SYSTEM                     \n"
                  << "=========================================================================================\n" 
                  << Color::RESET;

        std::cout << Color::BOLD << "🪑 SEAT MAP: [" 
                  << Color::GREEN << "■ Free (" << freeSeatIds.size() << ")" << Color::RESET << " | "
                  << Color::RED << "■ Occupied (" << activeBookings.size() << ")" << Color::RESET << " | "
                  << Color::YELLOW << "■ On Break" << Color::RESET << " | "
                  << Color::CYAN << "⏳ Waiting List: " << waitingQueue.size() << Color::RESET << "]\n";

        std::cout << "Zones: [S 1-20: Silent Zone] | [D 21-40: Discussion Pod] | [P 41-50: PC Lab]\n\n";

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                int sid = r * COLS + c + 1;
                if (freeSeatIds.find(sid) != freeSeatIds.end()) {
                    std::cout << Color::GREEN << "[" << std::setw(2) << sid << " Free ] " << Color::RESET;
                } else {
                    auto b = activeBookings[sid];
                    if (b->isOnBreak()) {
                        std::cout << Color::YELLOW << "[" << std::setw(2) << sid << " Brk " << b->getBreakRemainingMinutes() << "m] " << Color::RESET;
                    } else {
                        std::cout << Color::RED << "[" << std::setw(2) << sid << " " << std::setw(2) << b->getRemainingMinutes() << "m] " << Color::RESET;
                    }
                }
            }
            std::cout << "\n";
        }

        // Active Bookings Table
        std::cout << "\n" << Color::CYAN << "📋 ACTIVE BOOKINGS TABLE (" << activeBookings.size() << " Active):" << Color::RESET << "\n";
        std::cout << "-----------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(6) << "Seat" << std::setw(18) << "Name" << std::setw(14) << "Zone" 
                  << std::setw(12) << "Remaining" << std::setw(14) << "Status" << "Role\n";
        std::cout << "-----------------------------------------------------------------------------------------\n";

        std::map<int, std::shared_ptr<Booking>> sortedBookings(activeBookings.begin(), activeBookings.end());
        if (sortedBookings.empty()) {
            std::cout << "  No active reservations.\n";
        } else {
            for (auto it = sortedBookings.begin(); it != sortedBookings.end(); ++it) {
                int sid = it->first;
                const auto& b = it->second;
                std::string statusText = b->isOnBreak() ? ("On Break (" + std::to_string(b->getBreakRemainingMinutes()) + "m)") : "Active";
                std::cout << std::left << std::setw(6) << sid
                          << std::setw(18) << b->getUser()->getName().substr(0, 16)
                          << std::setw(14) << zoneToString(seatGrid[sid - 1]->getZone())
                          << std::setw(12) << (std::to_string(b->getRemainingMinutes()) + " mins")
                          << std::setw(14) << statusText
                          << b->getUser()->getUserRole() << "\n";
            }
        }
        std::cout << "-----------------------------------------------------------------------------------------\n";

        std::cout << Color::YELLOW << "📜 LIVE EVENT LOG:" << Color::RESET << "\n";
        for (auto it = eventAuditLog.begin(); it != eventAuditLog.end(); ++it) {
            std::cout << "  " << *it << "\n";
        }
        std::cout << "=========================================================================================\n";
    }
};

// ==========================================
// 8. INTERACTIVE MENU & THREAD LOOP
// ==========================================
int main() {
    SmartLibrarySystem library;
    std::atomic<bool> appRunning(true);

    // Background Timer Thread for seat & break auto-expiration
    std::thread timerThread([&]() {
        while (appRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            library.checkAndExpireBookings();
        }
    });

    int choice = 0;
    while (appRunning) {
        library.renderDashboard();
        std::cout << "\n" << Color::BOLD << "ACTIONS:" << Color::RESET << "\n"
                  << "1. 📌 Book a Single Seat\n"
                  << "2. 👥 Group Booking (Find Adjacent Seats)\n"
                  << "3. ⏸  Toggle Break / Return from Break (15-min Hold)\n"
                  << "4. 🔄 Vacate / Free Seat\n"
                  << "5. ⏳ Join Waiting Queue (When library is full)\n"
                  << "6. 🌲 Trie Auto-Complete Search by Name\n"
                  << "7. 🏆 View Study Hours Leaderboard\n"
                  << "8. ♻  Reset All Seats\n"
                  << "9. 📜 View Recent Operations (Stack)\n"
                  << "10. 🔄 Refresh Display\n"
                  << "11. 🚪 Exit\n"
                  << "Choose an option (1-11): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1) {
            int sid, duration, roleChoice;
            std::string name, mobile, entryTime;

            std::cout << "Enter Seat Number (1-50): ";
            std::cin >> sid;
            std::cin.ignore();
            std::cout << "Enter Student/Faculty Name: ";
            std::getline(std::cin, name);
            std::cout << "Enter Mobile Number: ";
            std::getline(std::cin, mobile);
            std::cout << "Enter Duration (minutes): ";
            std::cin >> duration;
            std::cin.ignore();
            std::cout << "Enter Entry Time (e.g. 14:00): ";
            std::getline(std::cin, entryTime);
            std::cout << "Role (1. Student, 2. Faculty): ";
            std::cin >> roleChoice;

            library.bookSeat(sid, name, mobile, duration, entryTime, roleChoice == 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        } else if (choice == 2) {
            int groupSize, duration;
            std::string leadName, mobile;
            std::cout << "Enter Group Size (2-10): ";
            std::cin >> groupSize;
            std::cin.ignore();
            std::cout << "Enter Group Leader Name: ";
            std::getline(std::cin, leadName);
            std::cout << "Enter Leader Mobile: ";
            std::getline(std::cin, mobile);
            std::cout << "Enter Duration (minutes): ";
            std::cin >> duration;

            library.bookGroupSeats(groupSize, leadName, mobile, duration);
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        } else if (choice == 3) {
            int sid;
            std::cout << "Enter Seat Number to Toggle Break: ";
            std::cin >> sid;
            library.toggleBreak(sid);
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        } else if (choice == 4) {
            int sid;
            std::cout << "Enter Seat Number to Vacate: ";
            std::cin >> sid;
            library.freeSeat(sid);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        } else if (choice == 5) {
            std::string name, mobile;
            int duration;
            std::cin.ignore();
            std::cout << "Enter Your Name: ";
            std::getline(std::cin, name);
            std::cout << "Enter Mobile: ";
            std::getline(std::cin, mobile);
            std::cout << "Requested Duration (minutes): ";
            std::cin >> duration;

            library.joinWaitingQueue(name, mobile, duration);
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));

        } else if (choice == 6) {
            std::string prefix;
            std::cin.ignore();
            std::cout << "Enter Name Prefix to Auto-Complete (e.g., 'Al', 'Joh'): ";
            std::getline(std::cin, prefix);
            library.searchStudentByName(prefix);
            std::cout << "\nPress Enter to return...";
            std::cin.get();

        } else if (choice == 7) {
            library.showLeaderboard();
            std::cout << "\nPress Enter to return...";
            std::cin.ignore();
            std::cin.get();

        } else if (choice == 8) {
            char confirm;
            std::cout << Color::RED << "Reset ALL seats? (y/n): " << Color::RESET;
            std::cin >> confirm;
            if (confirm == 'y' || confirm == 'Y') {
                library.resetAll();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

        } else if (choice == 9) {
            library.showRecentOperations();
            std::cout << "\nPress Enter to return...";
            std::cin.ignore();
            std::cin.get();

        } else if (choice == 10) {
            continue;

        } else if (choice == 11) {
            appRunning = false;
            std::cout << "\nShutting down Smart Library System...\n";
            break;
        }
    }

    if (timerThread.joinable()) {
        timerThread.join();
    }

    return 0;
}