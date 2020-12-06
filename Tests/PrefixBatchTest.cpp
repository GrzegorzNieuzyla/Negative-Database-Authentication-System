#include "PrefixBatchTest.h"
#include "../Generators/NDB_PrefixGenerator.h"
#include "../CLIInterface.h"

PrefixBatchTest::PrefixBatchTest(Solver solver) : GeneratorBatchTest(solver)
{

}

void PrefixBatchTest::Run(const std::string &filename)
{
    std::ofstream file(boost::ends_with(filename, ".csv") ? filename : filename + ".csv");
    file << FileUtils::CsvFileData::GetCsvDataHeader() << ";gen_time;solved;time;" << (_solver == GeneratorBatchTest::Solver::ZChaff ? "decisions" : "flips") << std::endl;
    auto settings  = CLIInterface::GetSettings();
    for (auto length : settings.recordLength)
    {
        for (auto count : settings.recordCount)
        {
            for (int i = 0; i < settings.repeat; ++i)
            {
                RunTest(file, count, length);
            }
        }
    }
}

void PrefixBatchTest::RunTest(std::ofstream& file, int count, int length)
{
    auto db = RandomValuesGenerator::GenerateRandomDB(count, length);
    NDB_PrefixGenerator generator(db, length);
    file << generator.GetCsvData().GetCsvRow();
    if (_solver == Solver::ZChaff)
    {
        auto result = ZChaffSolveTest(generator).Solve();
        file << ";" << result.genTime << ";" << result.solved << ";" << result.solveTime << ";" << result.decisions;
    }
    else if (_solver == Solver::WalkSat)
    {
        auto result = WalkSatSolveTest(generator).Solve();
        file << ";" << result.genTime << ";" << result.solved << ";" << result.solveTime << ";" << result.flips;
    }
    file << std::endl;
}
