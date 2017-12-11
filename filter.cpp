#include "filter.hpp"

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>

namespace FileSpreader
{
//#####################################################################################################################
    Filter::Filter(std::string const& filter)
        : filterRegex_{filter}
        , originalFilter_{}
        , initialized_{true}
    {
    }
//---------------------------------------------------------------------------------------------------------------------
    Filter::Filter()
        : filterRegex_{}
        , originalFilter_{}
        , initialized_{false}
    {
    }
//---------------------------------------------------------------------------------------------------------------------
    Filter::operator bool() const
    {
        return !initialized_;
    }
//---------------------------------------------------------------------------------------------------------------------
    Filter::operator std::string() const
    {
        return originalFilter_;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Filter::matches(std::string const& str) const
    {
        if (initialized_)
            return std::regex_match(str, filterRegex_);
        else
            return true;
    }
//#####################################################################################################################
    RegexFilter::RegexFilter(std::string const& filter)
        : Filter(RegexFilter::convertFilter(filter))
    {
        originalFilter_ = filter;
    }
//---------------------------------------------------------------------------------------------------------------------
    RegexFilter& RegexFilter::operator=(std::string const& filter)
    {
        filterRegex_.assign(RegexFilter::convertFilter(filter));
        originalFilter_ = filter;
        initialized_ = true;
        return *this;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool RegexFilter::matches(std::string const& str) const
    {
        return Filter::matches(str);
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string RegexFilter::convertFilter(std::string filter)
    {
        boost::replace_all(filter, "\\", "\\\\");
        boost::replace_all(filter, "^", "\\^");
        boost::replace_all(filter, ".", "\\.");
        boost::replace_all(filter, "$", "\\$");
        boost::replace_all(filter, "|", "\\|");
        boost::replace_all(filter, "(", "\\(");
        boost::replace_all(filter, ")", "\\)");
        boost::replace_all(filter, "[", "\\[");
        boost::replace_all(filter, "]", "\\]");
        boost::replace_all(filter, "*", "\\*");
        boost::replace_all(filter, "+", "\\+");
        boost::replace_all(filter, "?", "\\?");
        boost::replace_all(filter, "/", "\\/");

        return filter;
    }
//#####################################################################################################################
    WildcardFilter::WildcardFilter(std::string const& filter)
        : Filter(WildcardFilter::convertFilter(filter))
    {
        originalFilter_ = filter;
    }
//---------------------------------------------------------------------------------------------------------------------
    WildcardFilter& WildcardFilter::operator=(std::string const& filter)
    {
        filterRegex_.assign(WildcardFilter::convertFilter(filter));
        originalFilter_ = filter;
        initialized_ = true;
        return *this;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool WildcardFilter::matches(std::string const& str) const
    {
        return Filter::matches(str);
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string WildcardFilter::convertFilter(std::string filter)
    {
        boost::replace_all(filter, "\\", "\\\\");
        boost::replace_all(filter, "^", "\\^");
        boost::replace_all(filter, ".", "\\.");
        boost::replace_all(filter, "$", "\\$");
        boost::replace_all(filter, "|", "\\|");
        boost::replace_all(filter, "(", "\\(");
        boost::replace_all(filter, ")", "\\)");
        boost::replace_all(filter, "[", "\\[");
        boost::replace_all(filter, "]", "\\]");
        boost::replace_all(filter, "*", "\\*");
        boost::replace_all(filter, "+", "\\+");
        boost::replace_all(filter, "?", "\\?");
        boost::replace_all(filter, "/", "\\/");

        boost::replace_all(filter, "\\?", ".");
        boost::replace_all(filter, "\\*", ".*");

        return std::string("^") + filter + "$";
    }
//#####################################################################################################################
    bool wildcardFilter(std::string filter, std::string const& toMatch)
    {
        boost::replace_all(filter, "\\", "\\\\");
        boost::replace_all(filter, "^", "\\^");
        boost::replace_all(filter, ".", "\\.");
        boost::replace_all(filter, "$", "\\$");
        boost::replace_all(filter, "|", "\\|");
        boost::replace_all(filter, "(", "\\(");
        boost::replace_all(filter, ")", "\\)");
        boost::replace_all(filter, "[", "\\[");
        boost::replace_all(filter, "]", "\\]");
        boost::replace_all(filter, "*", "\\*");
        boost::replace_all(filter, "+", "\\+");
        boost::replace_all(filter, "?", "\\?");
        boost::replace_all(filter, "/", "\\/");

        boost::replace_all(filter, "\\?", ".");
        boost::replace_all(filter, "\\*", ".*");

        filter = std::string("^") + filter + "$";

        std::regex rgx(filter.c_str());

        return regex_match(toMatch, rgx);
    }
//---------------------------------------------------------------------------------------------------------------------
    bool regexFilter(std::string filter, std::string const& toMatch)
    {
        boost::replace_all(filter, "\\", "\\\\");
        boost::replace_all(filter, "^", "\\^");
        boost::replace_all(filter, ".", "\\.");
        boost::replace_all(filter, "$", "\\$");
        boost::replace_all(filter, "|", "\\|");
        boost::replace_all(filter, "(", "\\(");
        boost::replace_all(filter, ")", "\\)");
        boost::replace_all(filter, "[", "\\[");
        boost::replace_all(filter, "]", "\\]");
        boost::replace_all(filter, "*", "\\*");
        boost::replace_all(filter, "+", "\\+");
        boost::replace_all(filter, "?", "\\?");
        boost::replace_all(filter, "/", "\\/");

        std::regex rgx(filter.c_str());

        return regex_match(toMatch, rgx);
    }
//#####################################################################################################################
}
