#include "CLIInterface.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "Generators/NDB_PrefixGenerator.h"
#include "Generators/NDB_QHiddenGenerator.h"
#include "Generators/NDB_KHiddenGenerator.h"
#include "Generators/NDB_RandomizedGenerator.h"
#include "Generators/NDB_RandomizedOldGenerator.h"
#include "Generators/NDB_HybridGenerator.h"
#include "Generators/NDB_0HiddenGenerator.h"
#include "Streams/DimacsFileStream.h"
#include "Streams/NDBFileStream.h"
#include "Utils/Benchmark.h"


namespace po = boost::program_options;
namespace alg = boost::algorithm;

CLIInterface::CLIInterface(int argc, char* argv[])
{
    SetupCommandLine(argc, argv);
    SetupGenerator();
}

void CLIInterface::SetupGenerator()
{
    RandomValuesGenerator rng;
    alg::to_lower(_algorithm);

    if (_algorithm == alg::to_lower_copy(NDB_PrefixGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(_recordCount, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_PrefixGenerator(db, _recordLength));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_RandomizedOldGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(_recordCount, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_RandomizedOldGenerator(db, _recordLength));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_RandomizedGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(_recordCount, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_RandomizedGenerator(db, _recordLength));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_0HiddenGenerator::GetName()))
    {
        _generator = std::unique_ptr<NDB_Generator>(new NDB_0HiddenGenerator(_recordLength, _recordCountRatio, _specifiedBits));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_QHiddenGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(1, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_QHiddenGenerator(*db.begin(), _recordLength, _probabilityRatio, _recordCountRatio, _specifiedBits));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_KHiddenGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(1, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_KHiddenGenerator(*db.begin(), _recordLength, _probabilityRatios, _recordCountRatio, _specifiedBits));
    }
    else if (_algorithm == alg::to_lower_copy(NDB_HybridGenerator::GetName()))
    {
        auto db = rng.GenerateRandomDB(1, _recordLength);
        _generator = std::unique_ptr<NDB_Generator>(new NDB_HybridGenerator(*db.begin(), _recordLength, _probabilityRatio, _recordCountRatio));
    }
    else
    {
        std::cerr << "No such algorithm: " << _algorithm << std::endl;
        exit(1);
    }
}

void CLIInterface::SetupCommandLine(int argc, char* argv[])
{

    try{
        std::vector<std::string> availableGenerators = {
                NDB_PrefixGenerator::GetName(), NDB_RandomizedGenerator::GetName(),
                NDB_RandomizedOldGenerator::GetName(), NDB_0HiddenGenerator::GetName(),
                NDB_QHiddenGenerator::GetName(), NDB_KHiddenGenerator::GetName(),
                NDB_HybridGenerator::GetName()
        };
        auto algorithms = "Set algorithm (" + alg::join(availableGenerators, " | ") + ")";
        _description.add_options()
                ("record-count-ratio,r", po::value<double>(&_recordCountRatio), "Record count parameter")
                ("probability-ratio,q", po::value<double>(&_probabilityRatio), "Probability parameter")
                ("probability-ratios,p", po::value<std::vector<double>>(&_probabilityRatios)->multitoken(), "Probability parameters")
                ("record-length,l", po::value<int>(&_recordLength), "Record length")
                ("record-count,c", po::value<int>(&_recordCount), "Record count")
                ("specified-bits,k", po::value<int>(&_specifiedBits), "Specified bits count")
                ("output-file,o", po::value<std::string>(&_outputFile), "Output file")
                ("format,f", po::value<std::string>(&_generationMethod), "Output format (dimacs | ndb)")
                ("help,h", "Produce help message")
                ("algorithm,a", po::value<std::string>(&_algorithm)->required(), algorithms.c_str());

        po::store(po::parse_command_line(argc, argv, _description), _variablesMap);

        if (_variablesMap.count("help"))
        {
            std::cout << _description << "\n";
            exit(0);
        }
        po::notify(_variablesMap);

        if (_variablesMap.count("probability-ratios"))
        {
            _probabilityRatios = _variablesMap["probability-ratios"].as<std::vector<double>>();
        }

        if (_specifiedBits <= 0 || _recordLength <= 0 || _probabilityRatio < 0 || _probabilityRatio > 1 || _recordCountRatio <= 0)
        {
            std::cerr << "Incorrect parameter value" << std::endl;
            exit(1);
        }
    }
    catch (po::required_option& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    catch(...) {
        std::cout << _description << "\n";
        exit(0);
    }

}

void CLIInterface::Run()
{
    _generator->PrintParameters();
    std::unique_ptr<Stream> stream;
    alg::to_lower(_generationMethod);
    if (_outputFile.empty())
    {
        _outputFile = _algorithm + "_" +
                (_algorithm == NDB_PrefixGenerator::GetName() || _algorithm == NDB_RandomizedOldGenerator::GetName() ||
                 _algorithm == NDB_RandomizedGenerator::GetName() ? std::to_string(_recordCount) + "_" : "") +
                 std::to_string(_recordLength);
    }


    if (_generationMethod == "dimacs")
    {
        if (!alg::ends_with(_outputFile, ".dimacs")) _outputFile += ".dimacs";
        std::cout << "Format: Dimacs" << std::endl;
        std::cout << "Output: " << _outputFile << std::endl;
        stream = std::unique_ptr<Stream>(new DimacsFileStream(_outputFile));
    }
    else if (_generationMethod == "ndb")
    {
        if (!alg::ends_with(_outputFile, ".ndb")) _outputFile += ".ndb";
        std::cout << "Format: NDB" << std::endl;
        std::cout << "Output: " << _outputFile << std::endl;
        stream = std::unique_ptr<Stream>(new NDBFileStream(_outputFile));
    }
    else
    {
        std::cerr << "Unknown format: " << _generationMethod << std::endl;
        exit(1);
    }

    Benchmark benchmark;
    benchmark.Start();
    auto count = _generator->Generate(*stream);
    auto elapsed_time = benchmark.GetElapsedTime();
    std::cout << "Generated records: " << count << std::endl;
    std::cout << GetTimeElapsed(elapsed_time) << std::endl;

    if (_generationMethod == "dimacs")
    {
        dynamic_cast<DimacsFileStream*>(stream.get())->WriteHeader(_recordLength, count);
    }

}

std::string CLIInterface::GetTimeElapsed(std::size_t seconds)
{
    if (seconds < 60) return std::to_string(seconds) + "s";
    if (seconds < 3600) return std::to_string(seconds / 60) + "m" + std::to_string(seconds % 60) + "s";
    return std::to_string(seconds / 3600) + "h" + std::to_string((seconds % 3600) / 60) + "m" + std::to_string(seconds % 60) + "s";
}