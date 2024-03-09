#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <algorithm> // For using std::sort

// Class representing a bank account
class BankAccount {
    std::string accountNumber;
    float balance;

public:
    BankAccount() {}

    // Constructor for BankAccount with default value for balance
    BankAccount(std::string accountNumber, float balance = 0)
        : accountNumber(accountNumber), balance(balance) {}

    // Getter for account number
    std::string getAccountNumber() const {
        return this->accountNumber;
    }
};

// Interface for storing bank accounts
class IAccountStorage {
public:
    // Add an account
    virtual void addAccount(BankAccount account) = 0;
    // Find an account
    virtual BankAccount *findAccount(std::string accountNumber) = 0;
};

// Bank class handles accounts via the IAccountStorage interface
class Bank {
private:
    IAccountStorage *accountStorage;

public:
    // Constructor for the bank
    Bank(IAccountStorage *storage) : accountStorage(storage) {}

    // Add an account
    bool addAccount(std::string accountNumber) {
        accountStorage->addAccount(accountNumber);
        return true;
    }

    // Get an account
    BankAccount *getAccount(std::string accountNumber) {
        return accountStorage->findAccount(accountNumber);
    }
};

// Storage class using a std::map to store accounts
class MapAccountStorage : public IAccountStorage {
    std::map<std::string, BankAccount> accounts;

public:
    // Add an account to the map
    void addAccount(BankAccount account) override {
        accounts[account.getAccountNumber()] = account;
    }

    // Find an account in the map
    BankAccount *findAccount(std::string accountNumber) override {
        auto it = accounts.find(accountNumber);
        if (it != accounts.end())
            return &(it->second);
        else
            return nullptr;
    }
};

// Storage class using a vector and distributing accounts based on the first digit in the account number
class DistributedVectorAccountStorage : public IAccountStorage {
    std::vector<BankAccount> accounts[10]; // One vector for each first digit in the account number

    // Return reference to the correct vector based on the first digit in the account number
    std::vector<BankAccount> &getRef(char firstChar) {
        int index = firstChar - '0'; // Convert the character to an integer
        return accounts[index];
    }

public:
    // Add an account to the appropriate vector
    void addAccount(BankAccount account) override {
        char firstChar = account.getAccountNumber()[0]; // Get the first digit in the account number
        std::vector<BankAccount> &ref = getRef(firstChar); // Get reference to the appropriate vector
        ref.push_back(account); // Add the account to the vector
    }

    // Find an account by searching in the appropriate vector
    BankAccount *findAccount(std::string accountNumber) override {
        char firstChar = accountNumber[0]; // Get the first digit in the account number
        std::vector<BankAccount> &ref = getRef(firstChar); // Get reference to the appropriate vector
        for (BankAccount &account : ref) {
            if (account.getAccountNumber() == accountNumber) {
                return &account;
            }
        }
        return nullptr;
    }
};

// Storage class using binary search to find accounts
class BinarySearchStorage : public IAccountStorage {
    std::vector<BankAccount> accounts;

    // Sort the accounts based on the account number
    void sortAccounts() {
        std::sort(accounts.begin(), accounts.end(), [](const BankAccount &a, const BankAccount &b) {
            return a.getAccountNumber() < b.getAccountNumber();
        });
    }

public:
    // Add an account to the vector
    void addAccount(BankAccount account) override {
        accounts.push_back(account);
    }

    // Find an account using binary search
    BankAccount *findAccount(std::string accountNumber) override {
        sortAccounts(); // First, sort the accounts
        int left = 0, right = accounts.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            std::string midAccountNumber = accounts[mid].getAccountNumber();
            if (midAccountNumber == accountNumber)
                return &accounts[mid];
            else if (midAccountNumber < accountNumber)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return nullptr;
    }
};

// Templated Queue class with a static array, no reallocations
template<typename T, size_t Capacity>
class Queue {
private:
    T data[Capacity]; // Static array to store elements
    size_t start = 0; // Start position of the queue
    size_t end = 0; // End position of the queue
    size_t size = 0; // Current size of the queue

public:
    // Add an element to the queue
    // Returns true if successful, false if the queue is full
    bool push(const T &element) {
        if (size < Capacity) {
            data[end] = element;
            end = (end + 1) % Capacity; // Circular increment to avoid overflow
            ++size;
            return true;
        }
        return false; // Queue is full
    }

    // Remove and return the front element of the queue
    T pop() {
        T frontElement = data[start];
        start = (start + 1) % Capacity; // Circular increment to avoid overflow
        --size;
        return frontElement;
    }

    // Get the front element of the queue
    T front() const {
        return data[start];
    }

    // Get the size of the queue
    size_t getSize() const {
        return size;
    }

    // Check if the queue is empty
    bool isEmpty() const {
        return size == 0;
    }

    // Check if the queue is full
    bool isFull() const {
        return size == Capacity;
    }
};

int main(int, char **) {
    // Choose which type of storage to use
    DistributedVectorAccountStorage storage;
    // MapAccountStorage storage;
    // BinarySearchStorage storage;

    Bank bank(&storage); // Create a bank by type storage

    const int AntalAccounts = 1000000;

    std::string sFirst = "0000000001"; // First account number
    std::string sLast = std::to_string(AntalAccounts); // Last account number
    while (sLast.size() < 10) { // Pad with zeros in front to make the account number 10 characters long
        sLast = "0" + sLast;
    }
    std::string sNotFound = "notfound"; // An account number that does not exist

    std::cout << "INITIALIZE: " << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < AntalAccounts; i++) {
        std::string accountNumber = std::to_string(i + 1);
        while (accountNumber.size() < 10) { // Pad with zeros in front to make the account number 10 characters long
            accountNumber = "0" + accountNumber;
        }
        bank.addAccount(accountNumber); // Add the account to the bank
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "INIT Took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
              << " milliseconds" << std::endl;

    // Measure time to find the first acc
    startTime = std::chrono::high_resolution_clock::now();
    BankAccount *p = bank.getAccount(sFirst);
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << p->getAccountNumber() << " took: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count()
              << " nanoseconds" << std::endl;

    // Measure time to find the last acc
    startTime = std::chrono::high_resolution_clock::now();
    p = bank.getAccount(sLast);
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << p->getAccountNumber() << " took: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count()
              << " nanoseconds" << std::endl;

    // Measure time to find an acc that does not exist
    startTime = std::chrono::high_resolution_clock::now();
    p = bank.getAccount(sNotFound);
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "NOT FOUND" << " took: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count()
              << " nanoseconds" << std::endl;

    return 0;
}
