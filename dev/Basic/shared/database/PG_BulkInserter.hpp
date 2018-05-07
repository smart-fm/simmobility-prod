#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <libpq-fe.h>

namespace sim_mob
{

class PG_BulkInserter
{
public:
    PG_BulkInserter(const int numInsertsPerQuery);

    ~PG_BulkInserter();

    bool connect(const std::string& connectionStr);

    bool buildQuery(const std::string& tableName, const std::vector<std::string>& columnNames);

    bool setInputFile(const std::string& inputFile);

    bool bulkInsert();
private:
    std::ifstream* inputFile;

    std::string query;

    PGconn* connection;

    int numInsertsPerQuery;

    bool copyToDB(const std::string& buffer);
};

}
