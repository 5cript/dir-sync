#include "cloner_options.hpp"

namespace FileSpreader
{
//#####################################################################################################################
    void DestinationFilters::setRegexWhiteList(std::string const& rgx)
    {
        regexWhiteList_ = rgx;
    }
//---------------------------------------------------------------------------------------------------------------------
    void DestinationFilters::setRegexBlackList(std::string const& rgx)
    {
        regexBlackList_ = rgx;
    }
//---------------------------------------------------------------------------------------------------------------------
    void DestinationFilters::addWhiteListFilter(std::string const& filter)
    {
        whiteList_.push_back(filter);
    }
//---------------------------------------------------------------------------------------------------------------------
    void DestinationFilters::addBlackListFilter(std::string const& filter)
    {
        blackList_.push_back(filter);
    }
//---------------------------------------------------------------------------------------------------------------------
    void DestinationFilters::setBlackListFilter(std::vector <std::string> const& filter)
    {
        blackList_.clear();
        for (auto const& i : filter)
            blackList_.push_back(i);
    }
//---------------------------------------------------------------------------------------------------------------------
    void DestinationFilters::setWhiteListFilter(std::vector <std::string> const& filter)
    {
        whiteList_.clear();
        for (auto const& i : filter)
            whiteList_.push_back(i);
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <WildcardFilter> DestinationFilters::getBlackList() const
    {
        return blackList_;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <WildcardFilter> DestinationFilters::getWhiteList() const
    {
        return whiteList_;
    }
//---------------------------------------------------------------------------------------------------------------------
    RegexFilter DestinationFilters::getRegexBlackList() const
    {
        return regexBlackList_;
    }
//---------------------------------------------------------------------------------------------------------------------
    RegexFilter DestinationFilters::getRegexWhiteList() const
    {
        return regexWhiteList_;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool DestinationFilters::filtered(std::string const& path, WildcardFilter* additionalBlackList) const
    {
        using namespace std::string_literals;

        if (additionalBlackList != nullptr && additionalBlackList->matches(path))
            return false;

        for (auto const& black : blackList_)
            if (black.matches(path))
                return false;

        for (auto const& white : whiteList_)
            if (!white.matches(path))
                return false;

        if (regexBlackList_ && regexBlackList_.matches(path))
            return false;

        if (regexWhiteList_ && !regexWhiteList_.matches(path))
            return false;

        return true;
    }
//#####################################################################################################################
    bool ClonerOptions::isUsingArchiveBit() const
    {
        return useArchiveBit_;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string ClonerOptions::getTempSuffix() const
    {
        return temporarySuffix_;
    }
//---------------------------------------------------------------------------------------------------------------------
    DestinationFilters& ClonerOptions::getDestinationOptions(std::string const& destination)
    {
        return destinationOptions_[destination];
    }
//---------------------------------------------------------------------------------------------------------------------
    void ClonerOptions::setUseArchiveBit(bool useArchive)
    {
        useArchiveBit_ = useArchive;
    }
//---------------------------------------------------------------------------------------------------------------------
    void ClonerOptions::setTempSuffix(std::string const& suffix)
    {
        temporarySuffix_ = suffix;
    }
//#####################################################################################################################
    ClonerOptions ClonerOptionsFromMessage(Messages::Task const& taskMessage)
    {
        ClonerOptions options;

        if (taskMessage.useArchiveBit)
            options.setUseArchiveBit(taskMessage.useArchiveBit.get());
        else
            options.setUseArchiveBit(false);

        for (auto const& i : taskMessage.destinations)
        {
            auto& destOpts = options.getDestinationOptions(i.directory);
            destOpts.setBlackListFilter(i.getWholeBlackList());
            destOpts.setWhiteListFilter(i.getWholeWhiteList());

            if (i.blackListRegex)
                destOpts.setRegexBlackList(i.blackListRegex.get());
            if (i.whiteListRegex)
                destOpts.setRegexWhiteList(i.whiteListRegex.get());
        }

        return options;
    }
//#####################################################################################################################
}
