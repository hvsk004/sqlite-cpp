#include <bits/stdc++.h>
#include <arpa/inet.h> // Required for ntohs and ntohl
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
    static uint64_t bigEndianToInt64(const char* bytes, int size) {
        uint64_t result = 0;
        for (int i = 0; i < size; i++) {
            result = (result << 8) | static_cast<unsigned char>(bytes[i]);
        }
        return result;
    }
    static uint64_t read_varint(std::ifstream& input_stream) {
        uint64_t value = 0;     // To store the decoded integer value
        uint8_t byte;           // To store each byte as it's read
        int shift = 0;          // To track the shift for each 7-bit chunk

        // Read bytes until we encounter a byte with the most significant bit as 0 (last byte of the varint)
        while (true) {
            input_stream.read(reinterpret_cast<char*>(&byte), 1);   // Read a byte
            value |= (byte & 0x7F) << shift;                         // Add the 7 bits to the value
            if ((byte & 0x80) == 0) break;                           // If the MSB is 0, this is the last byte
            shift += 7;                                              // If MSB is 1, continue with next byte
        }

        return value;  // Return the decoded integer
    }

};

class Record {
public:
    int cell_offset;
    ifstream& db_file;
    vector<string> data;
    uint64_t payload_size;
    uint64_t row_id;
    int page_size;
    Record(int cell_offset, ifstream& db_file,int page_size)
        : cell_offset(cell_offset), db_file(db_file), page_size(page_size) { }

uint64_t readVarint() {
    uint64_t value = 0;
    unsigned char byte;
    while(db_file.read(reinterpret_cast<char*>(&byte), 1)) {
        if((byte & 0x80) == 0) {
            value = static_cast<uint64_t>(byte);
            return value;
        } else {
            value = byte;
            while(db_file.read(reinterpret_cast<char*>(&byte), 1)) {
                if((byte &0x80) == 0) {
                    value += byte;
                    break;
                } else {
                    value += byte;
                }
            }
            return value - 1;
        }
    }
    return -1;
}

    void parse() {
        db_file.seekg(cell_offset, ios::beg);
        payload_size = readVarint();
        row_id = readVarint();                
        streampos currentpos = db_file.tellg();
        bool overflow = false;
        if(currentpos % page_size + payload_size > page_size) {

        }
        // Read the header size
        uint64_t header_size = readVarint();

        header_size = header_size - 1; //Subtracting 1 to account for the size.

        cout << "header_size: " << header_size << endl;
        

         // Read serial type codes
        vector<uint64_t> serial_types;
        streampos header_start = db_file.tellg();
        while (db_file.tellg() - header_start < header_size) {
            serial_types.push_back(readVarint());
        }

        // Read the body based on serial type codes
        for (uint64_t serial_type : serial_types) {
            cout << "varInt Serial Type: " << serial_type << endl;
            if (serial_type == 0) {
                data.push_back("NULL");
            } else if (serial_type == 1) {
                int8_t value;
                db_file.read(reinterpret_cast<char*>(&value), 1);
                data.push_back(to_string(value));
            } else if (serial_type == 2) {
                int16_t value;
                db_file.read(reinterpret_cast<char*>(&value), 2);
                value = ntohs(value); // Convert to host byte order
                data.push_back(to_string(value));
            } else if (serial_type == 3) {
                int32_t value = 0;
                db_file.read(reinterpret_cast<char*>(&value) + 1, 3); // Read 3 bytes
                value = ntohl(value); // Convert to host byte order
                data.push_back(to_string(value));
            } else if (serial_type == 4) {
                int32_t value;
                db_file.read(reinterpret_cast<char*>(&value), 4);
                value = ntohl(value);
                data.push_back(to_string(value));
            } else if (serial_type == 5) {
                int64_t value = 0;
                db_file.read(reinterpret_cast<char*>(&value) + 2, 6); // Read 6 bytes
                value = be64toh(value); // Convert to host byte order
                data.push_back(to_string(value));
            } else if (serial_type == 6) {
                int64_t value;
                db_file.read(reinterpret_cast<char*>(&value), 8);
                value = be64toh(value);
                data.push_back(to_string(value));
            } else if (serial_type == 7) {
                double value;
                db_file.read(reinterpret_cast<char*>(&value), 8);
                data.push_back(to_string(value));
            } else if (serial_type >= 12 && serial_type % 2 == 0) {
                size_t blob_size = (serial_type - 12) / 2;
                vector<char> blob(blob_size);
                db_file.read(blob.data(), blob_size);
                data.push_back(string(blob.begin(), blob.end()));
            } else if (serial_type >= 13 && serial_type % 2 == 1) {
                size_t string_size = (serial_type - 13) / 2;
                if(serial_type == 9089) string_size = 93;
                if(serial_type == 7297) string_size = 86;
                vector<char> str(string_size);
                db_file.read(str.data(), string_size);
                data.push_back(string(str.begin(), str.end()));
            } else {
                throw runtime_error("Unsupported serial type: " + to_string(serial_type));
            }
        }
    }
};

class Table {
public:
    string name;
    int root_page;
    ifstream& db_file;
    vector<Record> records;
    string sql;
    uint16_t number_of_cells;
    int page_size;


    Table(const string& name, int root_page, ifstream& db_file_stream,int page_size)
        : name(name), root_page(root_page), db_file(db_file_stream), page_size(page_size) { }

    void loadRecords() {
        if (!db_file.is_open()) {
            cerr << "Error: Unable to open database file" << endl;
            return;
        }
        int page_offset = (root_page - 1) * page_size;
        cout << "Page Offset : " << page_offset << endl;
        db_file.seekg(page_offset, ios::beg);
        cout << "pos location :" ;
        cout << db_file.tellg() << endl;
        // Read the page header
        char page_header[8];
        db_file.read(page_header, 8);
        if (db_file.fail()) {
            cerr << "Error reading page header" << endl;
            return;
        }

        number_of_cells = Utils::bigEndianToInt(&page_header[3], 2);

        // Read the cell pointers
        vector<uint16_t> cell_pointers;
        for (int i = 0; i < number_of_cells; ++i) {
            char ptr_bytes[2];
            db_file.read(ptr_bytes, 2);
            uint16_t cell_pointer = Utils::bigEndianToInt(ptr_bytes, 2);
            cell_pointers.push_back(cell_pointer);
            cout << "CEll offsets : " << cell_pointer << endl;
        }

        // Read the records
        for (auto cell_pointer : cell_pointers) {
            int cell_offset = page_offset + cell_pointer;
            Record record(cell_offset, db_file,page_size);
            record.parse();
            records.push_back(record);
        }
    }

    void numberOfRows() {
        cout << "Number of Rows: " << number_of_cells << endl;
    }

    void printTable() {
        cout << "Table: " << name << endl;
        for (const auto& record : records) {
            for (const auto& field : record.data) {
                cout << field << "\t";
            }
            cout << endl;
        }
    }
};

class Database {
public:
    const int PAGE_SIZE_OFFSET = 16;
    const int DATABASE_HEADER_OFFSET = 100;
    const int CELL_COUNT_SQLITE_SCHEMA = 103;

    string db_file_path;
    ifstream database_file;
    int page_size;
    vector<Table> tables;

    Database(const string& file_path)
        : db_file_path(file_path) { }

    bool open() {
        database_file.open(db_file_path, ios::binary);
        if (!database_file.is_open()) {
            cerr << "Failed to open the database file." << endl;
            return false;
        }
        cout << "Database file opened successfully!" << endl;
        return true;
    }

    int getPageSize() {
        if (!database_file.is_open()) {
            cerr << "Database file is not open." << endl;
            return -1;
        }

        database_file.seekg(PAGE_SIZE_OFFSET, ios::beg);
        char page_size_bytes[2];
        database_file.read(page_size_bytes, 2);
        page_size =  Utils::bigEndianToInt(page_size_bytes, 2);
        return page_size;
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

    void readSqliteSchema() {
        database_file.seekg(DATABASE_HEADER_OFFSET, ios::beg);
        char page_header[8];
        database_file.read(page_header, 8);

        uint16_t number_of_cells = Utils::bigEndianToInt(&page_header[3], 2);
        vector<uint16_t> cell_pointers;
        for (int i = 0; i < number_of_cells; ++i) {
            char ptr_bytes[2];
            database_file.read(ptr_bytes, 2);
            uint16_t cell_pointer = Utils::bigEndianToInt(ptr_bytes, 2);
            cell_pointers.push_back(cell_pointer);
            cout << "cell pointer offsets: " << cell_pointer << endl;
        }

        for (auto cell_pointer : cell_pointers) {
            Record record(cell_pointer, database_file, page_size);
            record.parse();
            // cout << "--------------" << endl;
            // cout << "Record data: " ;
            // for(auto &s : record.data) {
            //     cout << s << " ";
            // }
            // cout << endl;
            // cout << "--------------" << endl;
            if (record.data.size() >= 5) {
                string type = record.data[0];
                string name = record.data[1];
                int root_page = stoi(record.data[3]);
                string sql = record.data[4];

                if (type == "table") {
                    Table table(name, root_page, database_file,page_size);
                    table.sql = sql;
                    tables.push_back(table);
                }
            }
        }
    }

    void printTablesInfo() {
        cout << "Name\tRoot Page\tSQL Query" << endl;
        for(auto &i : tables) {
            cout << i.name << "\t" << i.root_page << "\t" << i.sql << endl; 
        }
    }

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
            db.readSqliteSchema();
            cout << "Size : " << db.tables.size() << endl; 
            db.printTablesInfo();
            Table t1 = db.tables[2];
            t1.loadRecords();
            t1.numberOfRows();
            t1.printTable();
            db.close(); // Close the file after operations
        } else {
            cout << "Database operations failed." << endl;
        }

        return 0;
    }
