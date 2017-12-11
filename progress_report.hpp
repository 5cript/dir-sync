#pragma once

#include <vector>
#include <string>

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include "SimpleJSON/parse/jsd.hpp"
#   include "SimpleJSON/parse/jsd_convenience.hpp"
#   include "SimpleJSON/stringify/jss.hpp"
#   include "SimpleJSON/stringify/jss_fusion_adapted_struct.hpp"
#endif

#ifndef Q_MOC_RUN
#   include "SimpleXML/xmlify/xmlify.hpp"
#endif // Q_MOC_RUN

namespace FileSpreader
{
    struct DestinationProgress : public JSON::Stringifiable <DestinationProgress>
                               , public JSON::Parsable <DestinationProgress>
                               , public SXML::Xmlifiable <DestinationProgress>
    {
        std::string destination;
        std::string currentFile;
        std::vector <std::string> remainingFiles;
        uint64_t remainingFileCount;
        uint64_t scanFileCount;
        double currentFileProgress;
    };

    struct SourceGroupProgress : public JSON::Stringifiable <SourceGroupProgress>
                               , public JSON::Parsable <SourceGroupProgress>
                               , public SXML::Xmlifiable <SourceGroupProgress>
    {
        uint64_t sourceFileCount;
        std::vector <DestinationProgress> destinations;
    };

    struct ProgressReport : public JSON::Stringifiable <ProgressReport>
                          , public JSON::Parsable <ProgressReport>
                          , public SXML::Xmlifiable <ProgressReport>
    {
        uint64_t totalRemainingBytes;
        uint64_t totalRemainingFiles;
        std::vector <SourceGroupProgress> sources;
    };
}
BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::DestinationProgress,
    destination, currentFile, remainingFiles, currentFileProgress, remainingFileCount, scanFileCount
)

BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::SourceGroupProgress,
    destinations, sourceFileCount
)

BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::ProgressReport,
    totalRemainingBytes, totalRemainingFiles, sources
)
