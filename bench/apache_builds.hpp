//
// Copyright (c) 2024 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_BENCH_APACHE_BUILDS_HPP
#define BOOST_JSON_BENCH_APACHE_BUILDS_HPP

#include <boost/describe/class.hpp>
#include <boost/optional/optional.hpp>
#include <map>
#include <string>
#include <vector>

namespace boost {
namespace json {
namespace apache_builds {

struct label
{
    boost::optional<std::string> name;
};
BOOST_DESCRIBE_STRUCT( label, (), (name) )

struct job
{
    std::string name;
    std::string url;
    std::string color;
};
BOOST_DESCRIBE_STRUCT( job, (), (name, url, color) )

struct view
{
    std::string name;
    std::string url;
};
BOOST_DESCRIBE_STRUCT( view, (), (name, url) )

struct builds
{
    std::vector<label> assignedLabels;
    std::string mode;
    std::string nodeDescription;
    std::string nodeName;
    std::size_t numExecutors;
    std::string description;
    std::vector<job> jobs;
    std::map<std::string, int> overallLoad;
    view primaryView;
    bool quietingDown;
    std::size_t slaveAgentPort;
    std::map<std::string, int> unlabeledLoad;
    bool useCrumbs;
    bool useSecurity;
    std::vector<view> views;
};
BOOST_DESCRIBE_STRUCT(
    builds,
    (),
    ( assignedLabels,
      mode,
      nodeDescription,
      nodeName,
      numExecutors,
      description,
      jobs,
      overallLoad,
      primaryView,
      quietingDown,
      slaveAgentPort,
      unlabeledLoad,
      useCrumbs,
      useSecurity,
      views )
)

} // namespace apache_builds

struct apache_builds_support
{
    using type = apache_builds::builds;
    static constexpr char const* const name = "apache_builds.json";
};

} // namespace json
} // namespace boost

#endif // BOOST_JSON_BENCH_APACHE_BUILDS_HPP
