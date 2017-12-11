#include "task.hpp"

#include <boost/algorithm/string.hpp>

namespace FileSpreader { namespace Messages
{
//#####################################################################################################################
    std::vector <std::string> getWholeFilterList(boost::optional <std::vector <std::string>> const& filter,
                                                 boost::optional <std::string> const& colonList)
    {
        auto filters = filter ? filter.get() : std::vector <std::string>{};

        if (colonList && !colonList.get().empty())
            boost::split(filters, colonList.get(), boost::is_any_of(":"));

        return filters;
    }
//#####################################################################################################################
    std::vector <std::string> Destination::getWholeWhiteList() const
    {
        return getWholeFilterList(whiteList, whiteListString);
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <std::string> Destination::getWholeBlackList() const
    {
        return getWholeFilterList(blackList, blackListString);
    }
//#####################################################################################################################
    std::vector <std::string> Task::getDestinations() const
    {
        std::vector <std::string> res;

        for (auto const& i : destinations)
            res.push_back(i.directory);

        return res;
    }
//#####################################################################################################################
} // namespace Messages
} // namespace FileSpreader
