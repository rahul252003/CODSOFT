#include <iostream>
#include<windows.h>
#include <mysql.h>
#include <iomanip>
#include<stdlib.h>

#include <sstream>
#include <vector>
#include <ctime>
#include <unordered_map>

using namespace std;

class Book {
public:
    Book(string title, string author, string isbn, bool available = true)
        : title(title), author(author), isbn(isbn), available(available) {}

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }

    void checkOut() { available = false; }
    void returnBook() { available = true; }

private:
    string title;
    string author;
    string isbn;
    bool available;
};

class Borrower {
public:
    Borrower(string name, int id) : name(name), id(id) {}

    string getName() const { return name; }
    int getId() const { return id; }

private:
    string name;
    int id;
};

class Transaction {
public:
    Transaction(int borrowerId, string bookISBN, time_t checkoutTime)
        : borrowerId(borrowerId), bookISBN(bookISBN), checkoutTime(checkoutTime), returnTime(0) {}

    int getBorrowerId() const { return borrowerId; }
    string getBookISBN() const { return bookISBN; }
    time_t getCheckoutTime() const { return checkoutTime; }
    time_t getReturnTime() const { return returnTime; }

    void returnBook(time_t time) { returnTime = time; }

    bool isOverdue() const {
        if (returnTime == 0)
            return false;
        double diff = difftime(returnTime, checkoutTime) / (60 * 60 * 24);
        return diff > 14;
    }

private:
    int borrowerId;
    string bookISBN;
    time_t checkoutTime;
    time_t returnTime;
};

class Library {
public:
    Library() {
        connectToDatabase();
        loadBooks();
        loadBorrowers();
        loadTransactions();
    }

    ~Library() {
        delete con;
    }

    void addBook(const Book &book) {
        books.push_back(book);
        saveBook(book);
    }

    void addBorrower(const Borrower &borrower) {
        borrowers.push_back(borrower);
        saveBorrower(borrower);
    }

    void checkoutBook(int borrowerId, const string &isbn) {
        for (auto &book : books) {
            if (book.getISBN() == isbn) {
                if (book.isAvailable()) {
                    book.checkOut();
                    transactions.push_back(Transaction(borrowerId, isbn, time(nullptr)));
                    saveTransaction(borrowerId, isbn, time(nullptr));
                    cout << "Book checked out successfully.\n";
                    return;
                } else {
                    cout << "Book is currently unavailable.\n";
                    return;
                }
            }
        }
        cout << "Book not found.\n";
    }

    void returnBook(const string &isbn) {
        for (auto &book : books) {
            if (book.getISBN() == isbn) {
                if (!book.isAvailable()) {
                    book.returnBook();
                    for (auto &transaction : transactions) {
                        if (transaction.getBookISBN() == isbn && transaction.getReturnTime() == 0) {
                            transaction.returnBook(time(nullptr));
                            updateTransaction(isbn);
                            cout << "Book returned successfully.\n";
                            if (transaction.isOverdue()) {
                                cout << "The book is overdue. Please pay the fine.\n";
                            }
                            return;
                        }
                    }
                } else {
                    cout << "This book was not checked out.\n";
                    return;
                }
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByTitle(const string &title) const {
        for (const auto &book : books) {
            if (book.getTitle() == title) {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByAuthor(const string &author) const {
        for (const auto &book : books) {
            if (book.getAuthor() == author) {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByISBN(const string &isbn) const {
        for (const auto &book : books) {
            if (book.getISBN() == isbn) {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    vector<Book> books;
    vector<Borrower> borrowers;
    vector<Transaction> transactions;

    void connectToDatabase() {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("localhost", "root", "");
        con->setSchema("library");
    }

    void loadBooks() {
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("SELECT * FROM books");
        while (res->next()) {
            books.push_back(Book(res->getString("title"), res->getString("author"), res->getString("isbn"), res->getBoolean("available")));
        }
        delete res;
        delete stmt;
    }

    void saveBook(const Book &book) {
        sql::PreparedStatement *pstmt = con->prepareStatement("INSERT INTO books(title, author, isbn, available) VALUES (?, ?, ?, ?)");
        pstmt->setString(1, book.getTitle());
        pstmt->setString(2, book.getAuthor());
        pstmt->setString(3, book.getISBN());
        pstmt->setBoolean(4, book.isAvailable());
        pstmt->executeUpdate();
        delete pstmt;
    }

    void loadBorrowers() {
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("SELECT * FROM borrowers");
        while (res->next()) {
            borrowers.push_back(Borrower(res->getString("name"), res->getInt("id")));
        }
        delete res;
        delete stmt;
    }

    void saveBorrower(const Borrower &borrower) {
        sql::PreparedStatement *pstmt = con->prepareStatement("INSERT INTO borrowers(name, id) VALUES (?, ?)");
        pstmt->setString(1, borrower.getName());
        pstmt->setInt(2, borrower.getId());
        pstmt->executeUpdate();
        delete pstmt;
    }

    void loadTransactions() {
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("SELECT * FROM transactions");
        while (res->next()) {
            Transaction transaction(res->getInt("borrower_id"), res->getString("book_isbn"), res->getInt64("checkout_time"));
            transaction.returnBook(res->getInt64("return_time"));
            transactions.push_back(transaction);
        }
        delete res;
        delete stmt;
    }

    void saveTransaction(int borrowerId, const string &isbn, time_t checkoutTime) {
        sql::PreparedStatement *pstmt = con->prepareStatement("INSERT INTO transactions(borrower_id, book_isbn, checkout_time, return_time) VALUES (?, ?, ?, ?)");
        pstmt->setInt(1, borrowerId);
        pstmt->setString(2, isbn);
        pstmt->setInt64(3, checkoutTime);
        pstmt->setInt64(4, 0);
        pstmt->executeUpdate();
        delete pstmt;
    }

    void updateTransaction(const string &isbn) {
        sql::PreparedStatement *pstmt = con->prepareStatement("UPDATE transactions SET return_time = ? WHERE book_isbn = ? AND return_time = 0");
        pstmt->setInt64(1, time(nullptr));
        pstmt->setString(2, isbn);
        pstmt->executeUpdate();
        delete pstmt;
    }
};

void printMenu() {
    cout << "\nLibrary Management System\n";
    cout << "1. Add Book\n";
    cout << "2. Add Borrower\n";
    cout << "3. Search Book by Title\n";
    cout << "4. Search Book by Author\n";
    cout << "5. Search Book by ISBN\n";
    cout << "6. Checkout Book\n";
    cout << "7. Return Book\n";
    cout << "8. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    Library library;
    int choice;

    while (true) {
        printMenu();
        cin >> choice;

        if (choice == 1) {
            string title, author, isbn;
            cout << "Enter book title: ";
            cin.ignore();
            getline(cin, title);
            cout << "Enter book author: ";
            getline(cin, author);
            cout << "Enter book ISBN: ";
            getline(cin, isbn);
            library.addBook(Book(title, author, isbn));
            cout << "Book added successfully.\n";
        } else if (choice == 2) {
            string name;
            int id;
            cout << "Enter borrower name: ";
            cin.ignore();
            getline(cin, name);
            cout << "Enter borrower ID: ";
            cin >> id;
            library.addBorrower(Borrower(name, id));
            cout << "Borrower added successfully.\n";
        } else if (choice == 3) {
            string title;
            cout << "Enter book title: ";
            cin.ignore();
            getline(cin, title);
            library.searchBookByTitle(title);
        } else if (choice == 4) {
            string author;
            cout << "Enter book author: ";
            cin.ignore();
            getline(cin, author);
            library.searchBookByAuthor(author);
        } else if (choice == 5) {
            string isbn;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.searchBookByISBN(isbn);
        } else if (choice == 6) {
            int borrowerId;
            string isbn;
            cout << "Enter borrower ID: ";
            cin >> borrowerId;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.checkoutBook(borrowerId, isbn);
        } else if (choice == 7) {
            string isbn;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.returnBook(isbn);
        } else if (choice == 8) {
            break;
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}
