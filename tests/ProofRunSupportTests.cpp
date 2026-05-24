#include "ProofRunSupport.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    return { std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>() };
}
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "my-world-proof-run-support-test";
    std::filesystem::remove_all (root);

    const auto sourceRoot = root / "source";
    const auto outputRoot = root / "output";
    const auto relative = std::filesystem::path ("fixtures") / "sample.txt";
    std::filesystem::create_directories ((sourceRoot / relative).parent_path());
    {
        std::ofstream output (sourceRoot / relative, std::ios::binary);
        output << "proof support\n";
    }

    const auto candidates = myworld::proofCandidatePaths ({ sourceRoot, sourceRoot }, relative);
    expect (! candidates.empty(), "candidate list");
    expect (candidates.front() == sourceRoot / relative, "candidate root first");
    expect (std::count (candidates.begin(), candidates.end(), sourceRoot / relative) == 1,
            "deduplicates candidate roots");

    expect (myworld::createProofDirectoryIfMissing (outputRoot).empty(), "create directory");
    expect (std::filesystem::is_directory (outputRoot), "directory exists");

    const auto textPath = outputRoot / "nested" / "report.json";
    expect (myworld::writeProofTextFile (textPath, "{\"ok\":true}\n").empty(), "write text");
    expect (readTextFile (textPath) == "{\"ok\":true}\n", "read text");

    std::string error;
    const auto copiedPath = outputRoot / "copied" / "sample.txt";
    expect (myworld::copyFirstProofCandidate ({ sourceRoot }, relative, copiedPath, error), error);
    expect (readTextFile (copiedPath) == "proof support\n", "copied text");

    expect (myworld::clearProofDirectoryIfExists (outputRoot).empty(), "clear directory");
    expect (! std::filesystem::exists (outputRoot), "directory cleared");

    std::filesystem::remove_all (root);
    std::cout << "proof run support ok\n";
    return 0;
}
