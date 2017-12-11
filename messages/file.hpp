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
    struct File : public JSON::Stringifiable <File>
                , public JSON::Parsable <File>
    {
        std::string fileName;
    };
}
}

BOOST_FUSION_ADAPT_STRUCT
(
    FileSpreader::Messages::File,
    fileName
)
