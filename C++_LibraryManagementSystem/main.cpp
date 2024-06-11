#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <unordered_map>
// #include <bits/stdc++.h>

using namespace std;

class Book
{
public:
    Book(string title, string author, string isbn)
        : title(title), author(author), isbn(isbn), available(true) {}

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }

    void checkOut() { available = false; }
    void returnBook() { available = true; }

    string toCSV() const
    {
        return title + "," + author + "," + isbn + "," + (available ? "1" : "0");
    }

    static Book fromCSV(const string &str)
    {
        stringstream ss(str);
        string title, author, isbn, availableStr;
        getline(ss, title, ',');
        getline(ss, author, ',');
        getline(ss, isbn, ',');
        getline(ss, availableStr, ',');
        Book book(title, author, isbn);
        book.available = (availableStr == "1");
        return book;
    }

private:
    string title;
    string author;
    string isbn;
    bool available;
};
class Borrower
{
public:
    Borrower(string name, int id) : name(name), id(id) {}

    string getName() const { return name; }
    int getId() const { return id; }

    string toCSV() const
    {
        return name + "," + to_string(id);
    }

    static Borrower fromCSV(const string &str)
    {
        stringstream ss(str);
        string name;
        int id;
        getline(ss, name, ',');
        ss >> id;
        return Borrower(name, id);
    }

private:
    string name;
    int id;
};
class Transaction
{
public:
    Transaction(int borrowerId, string bookISBN, time_t checkoutTime)
        : borrowerId(borrowerId), bookISBN(bookISBN), checkoutTime(checkoutTime), returnTime(0) {}

    int getBorrowerId() const { return borrowerId; }
    string getBookISBN() const { return bookISBN; }
    time_t getCheckoutTime() const { return checkoutTime; }
    time_t getReturnTime() const { return returnTime; }

    void returnBook(time_t time) { returnTime = time; }

    bool isOverdue() const
    {
        if (returnTime == 0)
            return false;
        // Assuming a book is overdue if kept for more than 14 days
        double diff = difftime(returnTime, checkoutTime) / (60 * 60 * 24);
        return diff > 14;
    }

    string toCSV() const
    {
        return to_string(borrowerId) + "," + bookISBN + "," + to_string(checkoutTime) + "," + to_string(returnTime);
    }

    static Transaction fromCSV(const string &str)
    {
        stringstream ss(str);
        int borrowerId;
        string bookISBN;
        time_t checkoutTime, returnTime;
        ss >> borrowerId;
        ss.ignore(1, ',');
        getline(ss, bookISBN, ',');
        ss >> checkoutTime;
        ss.ignore(1, ',');
        ss >> returnTime;
        Transaction transaction(borrowerId, bookISBN, checkoutTime);
        transaction.returnTime = returnTime;
        return transaction;
    }

private:
    int borrowerId;
    string bookISBN;
    time_t checkoutTime;
    time_t returnTime;
};
class Library
{
public:
    Library()
    {
        loadBooks();
        loadBorrowers();
        loadTransactions();
    }

    ~Library()
    {
        saveBooks();
        saveBorrowers();
        saveTransactions();
    }

    void addBook(const Book &book)
    {
        books.push_back(book);
    }

    void addBorrower(const Borrower &borrower)
    {
        borrowers.push_back(borrower);
    }

    void checkoutBook(int borrowerId, const string &isbn)
    {
        for (auto &book : books)
        {
            if (book.getISBN() == isbn)
            {
                if (book.isAvailable())
                {
                    book.checkOut();
                    transactions.push_back(Transaction(borrowerId, isbn, time(nullptr)));
                    cout << "Book checked out successfully.\n";
                    return;
                }
                else
                {
                    cout << "Book is currently unavailable.\n";
                    return;
                }
            }
        }
        cout << "Book not found.\n";
    }

    void returnBook(const string &isbn)
    {
        for (auto &book : books)
        {
            if (book.getISBN() == isbn)
            {
                if (!book.isAvailable())
                {
                    book.returnBook();
                    for (auto &transaction : transactions)
                    {
                        if (transaction.getBookISBN() == isbn && transaction.getReturnTime() == 0)
                        {
                            transaction.returnBook(time(nullptr));
                            cout << "Book returned successfully.\n";
                            if (transaction.isOverdue())
                            {
                                cout << "The book is overdue. Please pay the fine.\n";
                            }
                            return;
                        }
                    }
                }
                else
                {
                    cout << "This book was not checked out.\n";
                    return;
                }
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByTitle(const string &title) const
    {
        for (const auto &book : books)
        {
            if (book.getTitle() == title)
            {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByAuthor(const string &author) const
    {
        for (const auto &book : books)
        {
            if (book.getAuthor() == author)
            {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void searchBookByISBN(const string &isbn) const
    {
        for (const auto &book : books)
        {
            if (book.getISBN() == isbn)
            {
                cout << "Book found: " << book.getTitle() << " by " << book.getAuthor() << " [ISBN: " << book.getISBN() << "]\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

private:
    vector<Book> books;
    vector<Borrower> borrowers;
    vector<Transaction> transactions;

    void loadBooks()
    {
        ifstream file("books.csv");
        string line;
        while (getline(file, line))
        {
            books.push_back(Book::fromCSV(line));
        }
        file.close();
    }

    void saveBooks()
    {
        ofstream file("books.csv");
        for (const auto &book : books)
        {
            file << book.toCSV() << endl;
        }
        file.close();
    }

    void loadBorrowers()
    {
        ifstream file("borrowers.csv");
        string line;
        while (getline(file, line))
        {
            borrowers.push_back(Borrower::fromCSV(line));
        }
        file.close();
    }

    void saveBorrowers()
    {
        ofstream file("borrowers.csv");
        for (const auto &borrower : borrowers)
        {
            file << borrower.toCSV() << endl;
        }
        file.close();
    }

    void loadTransactions()
    {
        ifstream file("transactions.csv");
        string line;
        while (getline(file, line))
        {
            transactions.push_back(Transaction::fromCSV(line));
        }
        file.close();
    }

    void saveTransactions()
    {
        ofstream file("transactions.csv");
        for (const auto &transaction : transactions)
        {
            file << transaction.toCSV() << endl;
        }
        file.close();
    }
};
void printMenu()
{
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

int main()
{
    Library library;
    int choice;

    while (true)
    {
        printMenu();
        cin >> choice;

        if (choice == 1)
        {
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
        }
        else if (choice == 2)
        {
            string name;
            int id;
            cout << "Enter borrower name: ";
            cin.ignore();
            getline(cin, name);
            cout << "Enter borrower ID: ";
            cin >> id;
            library.addBorrower(Borrower(name, id));
            cout << "Borrower added successfully.\n";
        }
        else if (choice == 3)
        {
            string title;
            cout << "Enter book title: ";
            cin.ignore();
            getline(cin, title);
            library.searchBookByTitle(title);
        }
        else if (choice == 4)
        {
            string author;
            cout << "Enter book author: ";
            cin.ignore();
            getline(cin, author);
            library.searchBookByAuthor(author);
        }
        else if (choice == 5)
        {
            string isbn;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.searchBookByISBN(isbn);
        }
        else if (choice == 6)
        {
            int borrowerId;
            string isbn;
            cout << "Enter borrower ID: ";
            cin >> borrowerId;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.checkoutBook(borrowerId, isbn);
        }
        else if (choice == 7)
        {
            string isbn;
            cout << "Enter book ISBN: ";
            cin.ignore();
            getline(cin, isbn);
            library.returnBook(isbn);
        }
        else if (choice == 8)
        {
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}
