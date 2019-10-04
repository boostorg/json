<img width="880" height = "80" alt = "Boost.JSON Title"
    src="https://raw.githubusercontent.com/vinniefalco/json/master/doc/images/repo-logo.png">

# Boost.JSON

Branch          | Travis | Appveyor | Azure Pipelines | codecov.io | Docs | Matrix |
:-------------: | ------ | -------- | --------------- | ---------- | ---- | ------ |
[`master`](https://github.com/vinniefalco/json/tree/master) | [![Build Status](https://travis-ci.org/vinniefalco/json.svg?branch=master)](https://travis-ci.org/vinniefalco/json) | [![Build status](https://ci.appveyor.com/api/projects/status/github/vinniefalco/json?branch=master&svg=true)](https://ci.appveyor.com/project/vinniefalco/fixed-string/branch/master) | [![Build Status](https://dev.azure.com/vinniefalco/fixed-string/_apis/build/status/pipeline?branchName=master)](https://dev.azure.com/vinniefalco/fixed-string/_build/latest?definitionId=6&branchName=master) | [![codecov](https://codecov.io/gh/vinniefalco/json/branch/master/graph/badge.svg)](https://codecov.io/gh/vinniefalco/json/branch/master) | [![Documentation](https://img.shields.io/badge/docs-master-brightgreen.svg)](http://www.boost.org/doc/libs/master/doc/html/json.html) | [![Matrix](https://img.shields.io/badge/matrix-master-brightgreen.svg)](http://www.boost.org/development/tests/master/developer/json.html)
[`develop`](https://github.com/vinniefalco/json/tree/develop) | [![Build Status](https://travis-ci.org/vinniefalco/json.svg?branch=develop)](https://travis-ci.org/vinniefalco/json) | [![Build status](https://ci.appveyor.com/api/projects/status/github/vinniefalco/json?branch=develop&svg=true)](https://ci.appveyor.com/project/vinniefalco/fixed-string/branch/develop) | [![Build Status](https://dev.azure.com/vinniefalco/fixed-string/_apis/build/status/pipeline?branchName=develop)](https://dev.azure.com/vinniefalco/fixed-string/_build/latest?definitionId=6&branchName=master) | [![codecov](https://codecov.io/gh/vinniefalco/json/branch/develop/graph/badge.svg)](https://codecov.io/gh/vinniefalco/json/branch/develop) | [![Documentation](https://img.shields.io/badge/docs-develop-brightgreen.svg)](http://www.boost.org/doc/libs/develop/doc/html/json.html) | [![Matrix](https://img.shields.io/badge/matrix-develop-brightgreen.svg)](http://www.boost.org/development/tests/develop/developer/json.html)

## This is currently **NOT** an official Boost library.

## Introduction

This library provides containers and algorithms which implement JSON
("JavaScript Object Notation"), a lightweight data-interchange format.
This format is easy for humans to read and write, and easy for machines
to parse and generate. It is based on a subset of the Javascript Programming
Language, Standard ECMA-262. JSON is a text format that is language-indepdent
but uses conventions that are familiar to programmers of the C-family of
languages, including C, C++, C#, Java, JavaScript, Perl, Python, and many
others. These properties make JSON an ideal data-interchange language.

## Motivation

A survey of existing JSON libraries shows impressive diversity and features.
However, no library is known to meet all of the design goals mentioned here.
In particular, we know of no library that supports incremental parsing and
serialization, and also supports custom allocators robustly.

## Design Goals

The design of the library achieves these goals:

* Robust support for custom allocators throughout.
* Array and object interfaces closely track their
  corresponding C++20 container equivalents.
* Use `std::basic_string` for strings.
* Minimize use of templates for reduced compilation times.
* Parsers and serializers work incrementally (['online algorithms]).
* Elements in objects may also be iterated in insertion order.

## License

Distributed under the Boost Software License, Version 1.0.
(See accompanying file [LICENSE_1_0.txt](LICENSE_1_0.txt) or copy at
https://www.boost.org/LICENSE_1_0.txt)
