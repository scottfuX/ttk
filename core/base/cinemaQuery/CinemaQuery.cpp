#include <CinemaQuery.h>

#if TTK_ENABLE_SQLITE3

    #include <sqlite3.h>

    // Process a row of a query result and append it to a string
    static int processRow(void *data, int argc, char **argv, char **azColName){
        int i;

        // Get output string as reference
        string& result = *((string*)data);

        // If string is empty then add a first row that records column names
        if(result==""){
            for(int i = 0; i<argc; i++)
                result+= (i>0?",":"") + string(azColName[i]);
            result+="\n";
        }

        // Append row content to string
        for(i = 0; i<argc; i++)
            result+= (i>0?",":"") + string(argv[i]);
        result+="\n";

        return 0;
    }
#endif

ttk::CinemaQuery::CinemaQuery(){}
ttk::CinemaQuery::~CinemaQuery(){}

string ttk::CinemaQuery::execute(
    const string& sqlTableDefinition,
    const string& sqlTableRows,
    const string& sqlQuery
) const{

    string result="";

    #if TTK_ENABLE_SQLITE3
        // SQLite Variables
        sqlite3* db;
        char* zErrMsg = 0;
        int rc;

        // Create Temporary Database
        {
            dMsg(cout, "[ttkCinemaQuery] Creating database ... ", timeMsg);
            Timer t;

            // Initialize DB in memory
            rc = sqlite3_open(":memory:", &db);
            if( rc!=SQLITE_OK ){
                stringstream msg;
                msg<<"failed\n[ttkCinemaQuery] ERROR: Unable to create database."<<endl;
                msg<<"[ttkCinemaQuery]         - "<< sqlite3_errmsg(db) <<endl;
                dMsg(cout, msg.str(), fatalMsg);
                return result;
            }

            // Create table
            rc = sqlite3_exec(db, sqlTableDefinition.data(), nullptr, 0, &zErrMsg);
            if( rc!=SQLITE_OK ){
                stringstream msg;
                msg<<"failed\n[ttkCinemaQuery] ERROR: " << zErrMsg <<endl;
                dMsg(cout, msg.str(), fatalMsg);

                sqlite3_free(zErrMsg);
                sqlite3_close(db);

                return result;
            }

            // Fill table
            rc = sqlite3_exec(db, sqlTableRows.data(), nullptr, 0, &zErrMsg);
            if( rc!=SQLITE_OK ){
                stringstream msg;
                msg<<"failed\n[ttkCinemaQuery] ERROR: " << zErrMsg <<endl;
                dMsg(cout, msg.str(), fatalMsg);

                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return result;
            } else {
                stringstream msg;
                msg << "done (" << t.getElapsedTime() << " s)."<< endl;
                dMsg(cout, msg.str(), timeMsg);
            }
        }

        // Run SQL statement on temporary database
        {
            dMsg(cout, "[ttkCinemaQuery] Querying database ... ", timeMsg);
            Timer t;

            // Perform query
            rc = sqlite3_exec(db, sqlQuery.data(), processRow, (void*)(&result), &zErrMsg);
            if( rc!=SQLITE_OK ){
                stringstream msg;
                msg<<"failed\n[ttkCinemaQuery] ERROR: " << zErrMsg <<endl;
                dMsg(cout, msg.str(), fatalMsg);

                sqlite3_free(zErrMsg);
                sqlite3_close(db);
                return result;
            } else {
                stringstream msg;
                msg << "done (" << t.getElapsedTime() << " s)." << endl;
                dMsg(cout, msg.str(), timeMsg);
            }
        }

        // Close database
        {
            dMsg(cout, "[ttkCinemaQuery] Closing  database ... ", timeMsg);

            // Delete DB
            rc = sqlite3_close(db);

            // Print status
            if(rc!=SQLITE_OK){
                stringstream msg;
                msg<<"failed\n[ttkCinemaQuery] ERROR: Unable to close database."<<endl;
                msg<<"[ttkCinemaQuery]         - "<< sqlite3_errmsg(db) <<endl;
                dMsg(cout, msg.str(), fatalMsg);
            } else {
                dMsg(cout, "done.\n", timeMsg);
            }
        }

    #else
        dMsg(cout, "[ttkCinemaQuery] ERROR: This filter requires Sqlite3.\n", fatalMsg);
    #endif

    return result;
}
