#pragma once

#include "filter.hpp"
#include "messages/task.hpp"

#include <vector>
#include <string>
#include <map>

namespace FileSpreader
{
    class DestinationFilters
    {
    public:
        // getters
        void setRegexWhiteList(std::string const& rgx);
        void setRegexBlackList(std::string const& rgx);

        void addWhiteListFilter(std::string const& filter);
        void addBlackListFilter(std::string const& filter);

        void setBlackListFilter(std::vector <std::string> const& filter);
        void setWhiteListFilter(std::vector <std::string> const& filter);

        std::vector <WildcardFilter> getBlackList() const;
        std::vector <WildcardFilter> getWhiteList() const;

        RegexFilter getRegexBlackList() const;
        RegexFilter getRegexWhiteList() const;

        /**
         *  Shall be filtered away?
         */
        bool filtered(std::string const& path, WildcardFilter* additionalBlackList) const;

    private:
        std::vector <WildcardFilter> blackList_ = {};
        std::vector <WildcardFilter> whiteList_ = {};

        RegexFilter regexWhiteList_ = {};
        RegexFilter regexBlackList_ = {};
    };

    class ClonerOptions
    {
    public:
        bool isUsingArchiveBit() const;
        std::string getTempSuffix() const;

        // setters
        DestinationFilters& getDestinationOptions(std::string const& destination);
        void setUseArchiveBit(bool useArchive);
        void setTempSuffix(std::string const& suffix);

    private:
        std::map <std::string, DestinationFilters> destinationOptions_ = {};
        std::string temporarySuffix_ = ".fs.temp"; // this will be implicitly black listed.
        bool useArchiveBit_ = false;
    };

    ClonerOptions ClonerOptionsFromMessage(Messages::Task const& taskMessage);
}
