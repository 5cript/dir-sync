#pragma once

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include "SimpleJSON/parse/jsd.hpp"
#   include "SimpleJSON/parse/jsd_convenience.hpp"
#   include "SimpleJSON/stringify/jss.hpp"
#   include "SimpleJSON/stringify/jss_fusion_adapted_struct.hpp"
#endif

#include <string>

namespace FileSpreader { namespace Messages
{
    struct Destination : public JSON::Stringifiable <Destination>,
                         public JSON::Parsable <Destination>
    {
        std::string directory;
        boost::optional <std::vector <std::string>> whiteList;
        boost::optional <std::vector <std::string>> blackList;
        boost::optional <std::string> whiteListString; // alternatively a colon seperated string of filters.
        boost::optional <std::string> blackListString; // alternatively a colon seperated string of filters.
        boost::optional <std::string> blackListRegex;
        boost::optional <std::string> whiteListRegex;

        std::vector <std::string> getWholeWhiteList() const;
        std::vector <std::string> getWholeBlackList() const;
    };

    struct Task : public JSON::Stringifiable <Task>,
                  public JSON::Parsable <Task>
    {
        std::string source;
        std::vector <Destination> destinations;
        boost::optional <bool> useArchiveBit;

        std::vector <std::string> getDestinations() const;
    };

    using TaskList = std::vector <Task>;
} // namespace Messages
} // namespace FileSpreader

BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::Messages::Destination,
    directory, whiteList, blackList, whiteListString, blackListString, blackListRegex, whiteListRegex
)

BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::Messages::Task,
    source, destinations, useArchiveBit
)
