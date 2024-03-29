#include "NDB_RandomizedOldGenerator.h"
#include "../Utils/NDBUtils.h"
#include <cmath>
#include <iostream>

NDB_RandomizedOldGenerator::NDB_RandomizedOldGenerator(const std::set<DBRecord>& db, int length)
 : NDB_Generator(db, length)
{}

std::size_t NDB_RandomizedOldGenerator::Generate(Stream& output)
{
    std::size_t count = 0;
    int init = ceil(log2(_length));
    auto W = NDBUtils::GetAllPatterns(init);

    for (int i = init; i < _length; ++i)
    {
        std::cout << i << std::endl;

        for (const auto& Vp : GetPatternsNotInDBWithPrefixes(W))
        {
            auto rand = RandomValuesGenerator::GetRandomInt(1, _length);
            for (auto j = 1; j <= rand; ++j)
            {
                output << PatternGenerate(Vp).ToString() << "\n";
                count++;
            }
        }
        W = GetPrefixes(i + 1);
    }
    return count;
}

NDBRecord NDB_RandomizedOldGenerator::PatternGenerate(const DBRecord &record) const
{
    auto permutation = RandomValuesGenerator::GenerateRandomPermutation(GetDBRecordSize());
    auto pattern = ToNDBRecord(record, GetDBRecordSize()).Characters();
    std::vector<std::pair<int, NDBChar>> SIV;
    for (int i = 0; i < pattern.size(); ++i)
    {
        auto permInd = permutation.GetIndex(i);
        auto bit = pattern[permInd];
        if (bit == NDBChar::Wildcard) continue;
        pattern[permInd] = NDBChar::Wildcard;
        if (!DoesNDBRecordMatchesAny(pattern))
        {
            SIV.emplace_back(permInd, bit);
        }
        else
        {
            pattern[permInd] = bit;
        }
    }
    int t = RandomValuesGenerator::GetRandomInt(0, SIV.size());
    auto choices = RandomValuesGenerator::GetRandomChoice(SIV, t);
    for (const auto& pair : choices)
    {
        pattern[pair.first] = pair.second;
    }


    return NDBRecord(pattern);
}

bool NDB_RandomizedOldGenerator::Matches(VectorView<NDBChar> ndbRecord, VectorView<bool> dbRecord)
{
    if (ndbRecord.Size() != dbRecord.Size())
        return false;
    auto ndbChar = ndbRecord._begin;
    for (const auto& dbChar : dbRecord)
    {
        if ((*ndbChar != NDBChar::Wildcard) && (dbChar && *ndbChar == NDBChar::Bit0) || (!dbChar && *ndbChar == NDBChar::Bit1))
            return false;
        ++ndbChar;
    }
    return true;
}

bool NDB_RandomizedOldGenerator::Matches(const std::vector<NDBChar>& ndbRecord, const std::vector<bool>& dbRecord)
{
    if (ndbRecord.size() != dbRecord.size())
        return false;
    for (int i = 0; i < ndbRecord.size();  ++i)
    {
        auto ndbChar = ndbRecord[i];
        auto dbChar = dbRecord[i];
        if (ndbChar == NDBChar::Wildcard) continue;
        if ((dbChar && ndbChar == NDBChar::Bit0) || (!dbChar && ndbChar == NDBChar::Bit1))
            return false;

    }
    return true;

}

bool NDB_RandomizedOldGenerator::Matches(const std::vector<NDBChar>& ndbRecord, const std::vector<bool>& dbRecord, std::vector<int> indices)
{
    if (ndbRecord.size() != dbRecord.size())
        return false;
    for (auto index : indices)
    {
        auto ndbChar = ndbRecord[index];
        auto dbChar = dbRecord[index];
        if (ndbChar == NDBChar::Wildcard) continue;
        if ((dbChar && ndbChar == NDBChar::Bit0) || (!dbChar && ndbChar == NDBChar::Bit1))
            return false;
    }
    return true;
}


bool NDB_RandomizedOldGenerator::DoesNDBRecordMatchesAny(std::vector<NDBChar> record) const
{
    if (record.empty()) return false;

    std::vector<int> indices;
    indices.reserve(record.size());
    for (int i = 0; i < record.size(); ++i)
    {
        if (record[i] != NDBChar::Wildcard)
        {
            indices.push_back(i);
        }
    }

    return std::any_of(_db.begin(), _db.end(), [&](const auto& dbRecord){
        return Matches(record, dbRecord.Characters(), indices);
    });
}

void NDB_RandomizedOldGenerator::PrintParameters() const
{
    std::cout << "Algorithm: " << GetName() << std::endl;
    std::cout << "c: " << _db.size() << std::endl;
    std::cout << "l: " << _length << std::endl;
}

FileUtils::CsvFileData NDB_RandomizedOldGenerator::GetCsvData() const
{
    return FileUtils::CsvFileData {.algorithm=GetName(), .length = _length, .count = static_cast<int>(_db.size())};
}



