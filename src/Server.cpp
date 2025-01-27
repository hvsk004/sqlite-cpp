#include <bits/stdc++.h>
using namespace std;

// Utility class for helper functions
class Utils {
public:
    // Function to convert a big-endian byte array to an integer
    static int bigEndianToInt(const char* bytes, int size) {
        int result = 0;
        for (int i = 0; i < size; i++) {
            result = (result << 8) | static_cast<unsigned char>(bytes[i]);
        }
        return result;
    }
};

// Database class to handle SQLite operations
class Database {
public:
    const int PAGE_SIZE_OFFSET = 16; // Offset for page size in SQLite database
    const int DATABASE_HEADER_OFFSET = 100; // Offset to the first page. This page contains the sqlite schema.
    const int CELL_COUNT_SQLITE_SCHEMA = 103; // Offset to find the number of tables
    string db_file;
    ifstream database_file; // File stream for database file

    // Constructor to initialize the database file path
    Database(const string &file_path) : db_file(file_path) {}

    // Method to open the database file
    bool open() {
        database_file.open(db_file, ios::binary);
        if (!database_file.is_open()) {
            cerr << "Failed to open the Database file." << endl;
            return false;
        }
        cout << "Database file opened successfully!" << endl;
        return true;
    }

    // Method to read the page size from the database file
    int getPageSize() {
        if (!database_file.is_open()) {
            cerr << "Database file is not open. Cannot read page size." << endl;
            return -1;
        }

        // Seek to the PAGE_SIZE_OFFSET (16 bytes)
        database_file.seekg(PAGE_SIZE_OFFSET, ios::beg);
        if (database_file.fail()) {
            cerr << "Failed to seek to PAGE_SIZE_OFFSET." << endl;
            return -1;
        }

        // Read 2 bytes for the page size
        char page_size_bytes[2];
        database_file.read(page_size_bytes, 2);
        if (database_file.fail()) {
            cerr << "Failed to read page size from the Database file." << endl;
            return -1;
        }

        // Use the utility function to convert big-endian bytes to integer
        return Utils::bigEndianToInt(page_size_bytes, 2);
    }

    //Method to get the Number of Tables (Cell Count)
    int getNumberOfTables() {
        if(!database_file.is_open()) {
            cerr << "Database file is not open. Cannot read page size." << endl;
            return -1;
        }
        /*
        100 Bytes -> Database Header
        3 Bytes -> 1 - BTree Page Type +  2 - 2 Byte Integer gives the start of first freelock on the page. 
        */
        database_file.seekg(CELL_COUNT_SQLITE_SCHEMA,ios::beg);

        // The cell count is stored in a 2 byte Integer.
        char cell_count_bytes[2];
        database_file.read(cell_count_bytes,2);
        if (database_file.fail()) {
            cerr << "Failed to read page size from the Database file." << endl;
            return -1;
        }

        //Convert the Big Endian bytes to integer.
        return Utils::bigEndianToInt(cell_count_bytes,2);
    }

    // Method to close the database file
    void close() {
        if (database_file.is_open()) {
            database_file.close();
        }
    }
};

int main() {
    string file_path = "sample.db";
    Database db(file_path);

    if (db.open()) {
        int page_size = db.getPageSize();
        if (page_size != -1) {
            cout << "Page Size: " << page_size << " bytes" << endl;
        } else {
            cout << "Failed to retrieve page size." << endl;
        }
        int numTables = db.getNumberOfTables();
        if(numTables != -1) {
            cout << "Number of Tables / Cell Count: " << numTables << endl;
        } else {
            cout << "Failed to retrieve number of tables / cell count." << endl;
        }

        db.close(); // Close the file after operations
    } else {
        cout << "Database operations failed." << endl;
    }

    return 0;
}
