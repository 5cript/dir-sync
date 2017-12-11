#pragma once

#include <regex>
#include <string>
#include <vector>

namespace FileSpreader
{
    class Filter
    {
    public:
        Filter(std::string const& filter);
        Filter();

        virtual ~Filter() = default;
        virtual bool matches(std::string const& str) const = 0;

        /**
         *  Returns if the filter is set.
         */
        explicit operator bool() const;
        operator std::string() const;

    protected:
        std::regex filterRegex_;
        std::string originalFilter_;
        bool initialized_;
    };

    class RegexFilter : public Filter
    {
    public:
        RegexFilter(std::string const& filter);
        RegexFilter() = default;

        RegexFilter& operator=(RegexFilter const&) = default;
        RegexFilter& operator=(std::string const& filter);

        bool matches(std::string const& str) const override;

    private:
        static std::string convertFilter(std::string filter);
    };

    class WildcardFilter : public Filter
    {
    public:
        WildcardFilter(std::string const& filter);
        WildcardFilter() = default;

        WildcardFilter& operator=(WildcardFilter const&) = default;
        WildcardFilter& operator=(std::string const& filter);

        bool matches(std::string const& str) const override;

    private:
        static std::string convertFilter(std::string filter);
    };
}
