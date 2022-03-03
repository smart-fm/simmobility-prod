#include "PG_BulkInserter.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;

PG_BulkInserter::PG_BulkInserter(const int numInsertsPerQuery) : numInsertsPerQuery(numInsertsPerQuery), query("")
{

}

PG_BulkInserter::~PG_BulkInserter()
{
    delete inputFile;
}

bool PG_BulkInserter::connect(const std::string &connectionStr)
{
    bool retVal = true;

    connection = PQconnectdb(connectionStr.c_str());

    if(PQstatus(connection) != CONNECTION_OK)
    {
        Print() << PQerrorMessage(connection);
        retVal = false;
    }

    return retVal;
}

bool PG_BulkInserter::buildQuery(const std::string& tableName, const std::vector<std::string>& columnNames)
{
    if (tableName.empty() || columnNames.empty())
    {
        return false;
    }

    query = "COPY " + tableName + "(";

    for (auto iter = columnNames.begin(); iter != columnNames.end()-1; ++iter)
    {
        query += *(iter);
        query += ",";
    }
    query += columnNames.back() + ") FROM STDIN DELIMITER ','";

    return true;
}

bool PG_BulkInserter::setInputFile(const std::string& fileName)
{
    inputFile = new std::ifstream(fileName);

    return inputFile->is_open();
}

bool PG_BulkInserter::bulkInsert()
{
    std::string streamBuf;
    std::string line;
    int numLines = 0;
    while(std::getline(*inputFile, line))
    {
        ++numLines;
        streamBuf.append(line);
        streamBuf.append("\n");
        if(numLines > numInsertsPerQuery)
        {
            copyToDB(streamBuf);
            streamBuf.clear();
            numLines = 0;
        }
    }

    return copyToDB(streamBuf);
}

bool PG_BulkInserter::copyToDB(const std::string& buffer)
{
    bool retVal = true;

    PGresult* res = PQexec(connection, query.c_str());

    if(PQresultStatus(res) != PGRES_COPY_IN)
    {
        Print() << "PG_BulkInserter: Copy Failed\n";
        retVal = false;
    }
    else
    {
        if(PQputCopyData(connection, buffer.c_str(), strlen(buffer.c_str())))
        {

            if (PQputCopyEnd(connection, NULL) == 1)
            {
                PGresult* res = PQgetResult(connection);
                if (PQresultStatus(res) != PGRES_COMMAND_OK)
                {
                    Print() << PQerrorMessage(connection);
                    retVal = false;
                }
            }
            else
            {
                Print() << PQerrorMessage(connection);
                retVal = false;
            }
        }
    }

    return retVal;
}
