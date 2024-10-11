//
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_EXAMPLE_CITM_CATALOG_HPP
#define BOOST_JSON_EXAMPLE_CITM_CATALOG_HPP

#include <boost/describe/class.hpp>
#include <boost/optional/optional.hpp>
#include <map>
#include <vector>

namespace boost {
namespace json {
namespace citm {

struct event
{
    std::nullptr_t description;
    unsigned long long id;
    boost::optional<std::string> logo;
    std::string name;
    std::vector<unsigned long long> subTopicIds;
    std::nullptr_t subjectCode;
    std::nullptr_t subtitle;
    std::vector<unsigned long long> topicIds;
};

BOOST_DESCRIBE_STRUCT(event, (), (description, id, logo, name, subTopicIds, subjectCode, subtitle, topicIds))

struct price
{
    unsigned amount;
    unsigned long long audienceSubCategoryId;
    unsigned long long seatCategoryId;
};

BOOST_DESCRIBE_STRUCT(price, (), (amount, audienceSubCategoryId, seatCategoryId))

struct area
{
    unsigned long long areaId;
    std::vector<unsigned long long> blockIds;
};

BOOST_DESCRIBE_STRUCT(area, (), (areaId, blockIds))

struct seat_category
{
    std::vector<area> areas;
    unsigned long long seatCategoryId;
};

BOOST_DESCRIBE_STRUCT(seat_category, (), (areas, seatCategoryId))

struct performance
{
    unsigned long long eventId;
    unsigned long long id;
    boost::optional<std::string> logo;
    std::nullptr_t name;
    std::vector<price> prices;
    std::vector<seat_category> seatCategories;
    std::nullptr_t seatMapImage;
    unsigned long long start;
    std::string venueCode;
};

BOOST_DESCRIBE_STRUCT(performance, (), (eventId, id, logo, name, prices, seatCategories, seatMapImage, start, venueCode))

struct catalog
{
    std::map<std::string, std::string> areaNames;
    std::map<std::string, std::string> audienceSubCategoryNames;
    std::map<std::string, std::string> blockNames;
    std::map<std::string, event> events;
    std::vector<performance> performances;
    std::map<std::string, std::string> seatCategoryNames;
    std::map<std::string, std::string> subTopicNames;
    std::map<std::string, std::string> subjectNames;
    std::map<std::string, std::string> topicNames;
    std::map<std::string, std::vector<unsigned long long>> topicSubTopics;
    std::map<std::string, std::string> venueNames;
};

BOOST_DESCRIBE_STRUCT(catalog, (),
    (areaNames, audienceSubCategoryNames, blockNames, events, performances,
    seatCategoryNames, subTopicNames, subjectNames, topicNames, topicSubTopics, venueNames))

} // namespace citm

struct citm_catalog_support
{
    using type = citm::catalog;
    static constexpr char const* const name = "citm_catalog.json";
};

} // namespace json
} // namespace boost

#endif // BOOST_JSON_EXAMPLE_CITM_CATALOG_HPP
