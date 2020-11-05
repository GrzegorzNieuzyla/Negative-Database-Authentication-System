#pragma once
#include <vector>
#include "../NDB.h"
#include "../DB.h"
#include <unordered_set>
#include <set>
#include <random>



class NDBGenerator
{
public:
    NDBGenerator(const std::set<DBRecord>& db, int length);
    virtual NDB Generate() = 0;

protected:
    [[nodiscard]] std::vector<DBRecord> GetPatternsNotInDbWithPrefix(DBRecord prefix) const;
    [[nodiscard]] std::vector<DBRecord> GetPatternsNotInDBWithPrefixes(const std::vector<DBRecord>& Wi) const;

    // Converts DB records prefix to NDB record, pads to 'len' length with '*'
    static NDBRecord ToNDBRecord(const DBRecord& record, std::size_t len);

    // Gets all prefixes of length 'len' from DB
    [[nodiscard]] std::vector<DBRecord> GetPrefixes(std::size_t len) const;

    // Returns iterator bounds of records where 'prefix' is a prefix to
    [[nodiscard]] static std::pair<std::vector<DBRecord>::const_iterator, std::vector<DBRecord>::const_iterator>
    FindPrefixBounds(const std::vector<DBRecord>& db, const DBRecord& prefix);

    int GetRandomValue(int lower, int upper);

    std::vector<DBRecord> _db;
    int _length;
    std::mt19937 _rng;
};
