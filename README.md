# Basic SQLite Clone in C++

## Project Overview
This project aims to create a basic version of SQLite in C++ that can perform essential database operations on `.db` files. The implementation focuses on understanding the structure of SQLite databases and interacting with them. The application supports the following features:

- Reading `.db` files
- Retrieving the number of tables
- Determining the page size
- Counting rows in a table
- Reading data from a single column
- Reading data from multiple columns
- Filtering data with a `WHERE` clause
- Performing full-table scans
- Retrieving data using an index

## File Structure
The entire project is implemented in a single file:
- **server.cpp**: Contains all the classes, utility functions, and logic for interacting with SQLite `.db` files.

## Classes

### 1. **Database**
This class represents the SQLite database and handles operations like file opening, reading headers, and general database metadata operations.

#### Key Methods:
- **open()**: Opens the `.db` file in binary mode.
- **getPageSize()**: Reads the page size from the database header.
- **getTableCount()**: Calculates the number of tables in the database.

### 2. **Table**
This class focuses on table-level operations, such as row counts and column data retrieval.

#### Key Methods:
- **countRows()**: Counts the number of rows in a specific table.
- **readColumn()**: Reads data from a single column.
- **readMultipleColumns()**: Reads data from multiple columns in a table.

### 3. **Query**
This class processes and executes queries on the database.

#### Key Methods:
- **filterData()**: Filters data using a `WHERE` clause.
- **fullTableScan()**: Retrieves all data from a table.
- **retrieveUsingIndex()**: Retrieves data using an index.

### 4. **Utils**
A utility class containing helper functions for general tasks like byte conversions and parsing.

#### Key Methods:
- **bigEndianToInt(const char* bytes, int size)**: Converts big-endian byte arrays to integers.
- **parseString(const char* bytes, int size)**: Converts byte arrays to strings.

## Project Workflow

1. **Open Database File**: Use the `Database::open()` method to open and validate the SQLite `.db` file.
2. **Retrieve Metadata**: Call `Database::getPageSize()` and `Database::getTableCount()` to understand the structure of the database.
3. **Work with Tables**: Use the `Table` class to count rows and fetch column data.
4. **Execute Queries**: Use the `Query` class for full-table scans, `WHERE` clauses, and index-based retrievals.
5. **Utility Functions**: Utilize the `Utils` class for data conversions and parsing during database operations.

## Example Usage
```cpp
#include "server.cpp"

int main() {
    string db_path = "example.db";
    Database db(db_path);

    if (db.open()) {
        cout << "Page Size: " << db.getPageSize() << " bytes" << endl;
        cout << "Number of Tables: " << db.getTableCount() << endl;

        Table table("example_table");
        cout << "Row Count: " << table.countRows(db) << endl;

        Query query;
        query.fullTableScan(db, "example_table");
    } else {
        cout << "Failed to open database file." << endl;
    }

    return 0;
}
```

## How to Run
1. Clone the repository.
2. Compile `server.cpp` using a C++ compiler (e.g., `g++`):
   ```sh
   g++ -o sqlite_clone server.cpp
   ```
3. Run the executable with the path to an SQLite `.db` file:
   ```sh
   ./sqlite_clone
   ```

## Future Enhancements
- Implement support for writing data to the database.
- Add support for complex SQL queries.
- Include better error handling and validation for `.db` files.
- Support additional SQLite features, such as transactions and schemas.

## Dependencies
This project is built using standard C++ libraries and does not require any external dependencies.

## Notes
- Ensure the `.db` file is a valid SQLite database.
- Currently, the project only supports reading and basic querying.


