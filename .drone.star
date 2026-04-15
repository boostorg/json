# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.
# Copyright (c) 2026 Dmitry Arkhipov (grisumbras@yandex.ru)


def main(ctx):
    globalenv={'B2_CI_VERSION': '1', 'B2_FLAGS': 'warnings=extra warnings-as-errors=on', 'DEPINST': '--reject-cycles'}
    with_stdxxfs = { 'B2_LINKFLAGS': '-lstdc++fs', }
    cmake_env = { 'CMAKE_VERSION': '3.8.2'}

    return generate(
        ['gcc >=4.9', 'clang >=3.8', 'msvc >=14.0', 'apple-clang >=13.4'],
        '>=11',
        globalenv=globalenv,
        overrides=[
            { 'match': {'compiler': 'gcc ~8'},
              'environment': with_stdxxfs,
            },
            { 'match': {'compiler': 'clang >=6 <9'},
              'environment': with_stdxxfs,
            },
            { 'match': {'compiler': 'gcc =4.9'},
              'name': 'GCC 4.9 C++11 (no op)',
              'environment': {'B2_CXXSTD': '11', 'B2_FLAGS': '--with-json'},
            },
            { 'match': {'compiler': 'gcc <12', 'architecture': 's390x'},
              'skip': True,
            },
            { 'match': {'compiler': 'gcc <12', 'architecture': 'arm64'},
              'skip': True,
            },
            { 'match': {'compiler': 'gcc <11', 'os': 'freebsd'},
              'skip': True,
            },
            { 'match': {'compiler': 'clang <15', 'os': 'freebsd'},
              'skip': True,
            },
        ],
        extras=[
            { 'match': {'compiler': 'gcc =9', 'os': 'linux'},
              'name': 'GCC 9 CMake superproject',
              'type': 'cmake-superproject',
              'environment': cmake_env,
            },
            { 'match': {'compiler': 'gcc =9', 'os': 'linux'},
              'name': 'GCC 9 CMake main project',
              'type': 'cmake-mainproject',
              'environment': cmake_env,
            },
            { 'match': {'compiler': 'gcc =9', 'os': 'linux'},
              'name': 'GCC 9 CMake subdirectory',
              'type': 'cmake-subdirectory',
              'environment': cmake_env,
            },
            { 'match': {'compiler': 'msvc =14.5'},
              'name': 'MSVC 14.5 CMake superproject',
              'type': 'cmake-superproject',
            },
            { 'match': {'compiler': 'msvc =14.5'},
              'name': 'MSVC 14.5 CMake main project',
              'type': 'cmake-mainproject',
            },
            { 'match': {'compiler': 'msvc =14.5'},
              'name': 'MSVC 14.5 CMake subdirectory',
              'type': 'cmake-subdirectory',
            },
            { 'match': {'compiler': 'gcc =latest', 'os': 'linux'},
              'name': 'GDB printers',
              'packages' : ['gdb'],
              'environment': {
                  'B2_TARGETS': 'libs/json/test//gdb-printers',
                  'B2_VARIANT': 'debug',
                  'B2_CXXSTD': '23',
              },
            },
            { 'match': {'compiler': 'gcc =latest', 'os': 'linux'},
              'name': 'Codecov',
              'type': 'codecov',
              'environment': {
                  'COMMENT': 'codecov.io',
                  'LCOV_BRANCH_COVERAGE': '0',
                  'B2_CXXSTD': '23',
                  'B2_VARIANT': 'debug',
                  'B2_DEFINES': 'BOOST_JSON_EXPENSIVE_TESTS',
                  'CODECOV_TOKEN': {'from_secret': 'codecov_token'},
              },
            },
            { 'match': {'compiler': 'gcc =latest', 'os': 'linux'},
              'special': 'asan',
            },
            { 'match': {'compiler': 'gcc =latest', 'os': 'linux'},
              'special': 'tsan',
            },
            { 'match': {'compiler': 'gcc =latest', 'os': 'linux'},
              'special': 'ubsan',
            },
            { 'match': {'compiler': 'clang =latest', 'os': 'linux'},
              'special': 'asan',
            },
            # { 'match': {'compiler': 'clang =latest', 'os': 'linux'},
            #   'special': 'tsan',
            # },
            { 'match': {'compiler': 'clang =latest', 'os': 'linux'},
              'special': 'ubsan',
            },
            { 'match': {'compiler': 'clang =latest', 'os': 'linux'},
              'special': 'valgrind',
            },
        ],
    ) + [
        linux_cxx("docs", "g++", buildtype="docs", buildscript="drone", image="cppalliance/boost_superproject_build:24.04-v3", environment={'COMMENT': 'docs'}, globalenv=globalenv),
    ]

def generate(
    compilers,
    cxxstd_range,
    cxxstd_window=3,
    oses=["linux", "windows", "macos", 'freebsd'],
    architectures=["x86_64", "arm64", "s390x"],
    script='drone',
    globalenv={},
    overrides=[],
    extras=[],
):
    # Standards we should test whenever possible
    cpp_versions = cxxstd_in_range(cxxstd_range)

    jobs = []
    for compiler_spec in compilers:
        for [compiler, version] in get_requested_compilers(compiler_spec):
            comp = _supported_compilers.get(compiler) or {}
            comp_v = comp.get(version)

            custom_cpp = comp_v.get('custom_cpp') or {}

            cxxstd  = []
            for v in cpp_versions_available(compiler, version, cpp_versions):
                cxxstd.append(custom_cpp.get(v) or str(v)[-2:])
            if len(cxxstd) > cxxstd_window:
                cxxstd = cxxstd[-cxxstd_window:]
            cxxstd = ','.join(cxxstd)

            for os in oses:
                for arch in architectures:
                    platform = comp_v.get('platforms').get((os, arch))
                    if platform:
                        job = {
                            'compiler': compiler,
                            'version': version,
                            'os': os,
                            'architecture': arch,
                            'type': 'boost',
                            'script': script,
                        }
                        job.update(platform)

                        name = comp.get('display_name') or compiler
                        name += ' ' + version
                        if arch != 'x86_64':
                            name += ' (' + arch + ')'
                        job['name'] = name

                        if compiler == 'gcc':
                            job['cxx'] = 'g++-' + version
                        elif compiler == 'clang':
                            job['cxx'] = 'clang++-' + version

                        job['environment'] = {
                            'B2_TOOLSET': compiler + '-' + version,
                            'B2_CXXSTD': cxxstd,
                            'B2_VARIANT': 'debug,release',
                        }
                        job['environment'].update(
                            platform.get('environment', {}))

                        override_job(job, overrides)
                        if not job.get('skip'):
                            jobs.append(job)

                        for extra in extra_jobs(job, extras):
                            jobs.append(extra)

    result = []
    # Append job
    for job in jobs:
        name = job['name']
        buildtype = job['type']
        buildscript = job['script']
        cxx = job.get('cxx', '')
        environment = job.get('environment', {})

        packages = ' '.join(job.get('packages', []))

        arch = job['architecture']
        arch = {'x86_64': 'amd64'}.get(arch) or arch

        privileged = job.get('privileged')

        if job['os'] == 'windows':
            job_spec = windows_cxx(
                job['name'],
                '',
                image=job['image'],
                buildtype=buildtype,
                buildscript=buildscript,
                environment=environment,
                globalenv=globalenv)
        elif job['os'] == 'macos':
            job_spec = osx_cxx(
                job['name'],
                cxx,
                packages=packages,
                buildscript=buildscript,
                buildtype=buildtype,
                xcode_version=job.get('xcode_version'),
                environment=environment,
                globalenv=globalenv)
        elif job['os'] == 'freebsd':
            job_spec = freebsd_cxx(
                name,
                cxx,
                buildscript=buildscript,
                buildtype=buildtype,
                freebsd_version="13.1",
                environment=environment,
                globalenv=globalenv)
        elif job['os'] == 'linux':
            job_spec = linux_cxx(
                name,
                cxx,
                packages=packages,
                llvm_os=job.get('llvm_os'),
                llvm_ver=job.get('llvm_ver'),
                buildscript=buildscript,
                buildtype=buildtype,
                image=job['image'],
                environment=environment,
                arch=arch,
                globalenv=globalenv,
                privileged=privileged)

        result.append(job_spec)

    return result

def cxxstd_in_range(ranges):
    supported_cxx = [1998, 2003, 2011, 2014, 2017, 2020, 2023, 2026]
    cxxs = []

    # Replace with 4 digit major so semver makes sense
    if type(ranges) == 'string':
        ranges = parse_semver_range(ranges)
    disjunction = ranges

    for conjunction in disjunction:
        for comparator in conjunction:
            if comparator[1] < 98:
                comparator[1] += 2000
            elif comparator[1] < 100:
                comparator[1] += 1900

    # Extract matching versions
    for v in supported_cxx:
        if version_match((v, 0, 0), disjunction):
            cxxs.append(v)
    return cxxs

def parse_compiler_request(request):
    parts = request.strip().split(' ', 1)
    name = parts[0]
    version_spec = parts[1].strip() if len(parts) > 1 else '>0'
    if version_spec == '=latest':
        version = parse_semver(_supported_compilers[name]['versions'][0])
        version.insert(0, '=')
        return (name, [[version]])
    else:
        return (name, parse_semver_range(version_spec))

def get_requested_compilers(request):
    name, parsed_spec = parse_compiler_request(request)
    compilers = []
    for version in _supported_compilers[name]['versions']:
        if version_match(version, parsed_spec):
            compilers.append((name, version))
    return compilers

def cpp_versions_available(compiler, version, cpp_versions):
    latest = _supported_compilers[compiler][version]['latest_cpp']
    result = []
    for v in cpp_versions:
        if v <= latest:
            result.append(v)
    return result

def override_job(job, overrides):
    for item in overrides:
        if not override_matches(job, item):
            continue
        for k, v in item.items():
            if k == 'match':
                continue

            merge_values(job, k, v)

def extra_jobs(job, extras):
    result = []
    for item in extras:
        if not override_matches(job, item):
            continue
        new_job = {}
        for k, v in job.items():
            if type(v) == 'dict':
                new_job[k] = {}
                new_job[k].update(v)
            elif type(v) == 'list':
                new_job[k] = list(v)
            else:
                new_job[k] = v

        apply_special(new_job, item.get('special'))

        for k, v in item.items():
            if k in ('match', 'special'):
                continue
            merge_values(new_job, k, v)

        result.append(new_job)
    return result

def merge_values(d, k, v):
    d_v = d.get(k)
    if (type(d_v) == 'dict') and (type(v) == 'dict'):
        for v_k, v_v in v.items():
            d_v[v_k] = v_v
    elif (type(d_v) == 'list') and (type(v) == 'list'):
        d_v.extend(v)
    else:
        d[k] = v

def override_matches(job, override):
    for k, v in override.get('match').items():
        if k == 'compiler':
            compiler, version_spec = parse_compiler_request(v)
            if job.get('compiler') != compiler:
                return False
            if not version_match(job.get('version'), version_spec):
                return False
        elif job.get(k) != v:
            return False
    return True

def set_or_append(obj, key, value):
    cur_value = obj.get(key, '')
    if cur_value:
        cur_value += ' ' + value
    else:
        cur_value = value
    obj[key] = value

def apply_special(job, special):
    if special == 'asan':
        job['name'] = 'ASan ' + job['name']
        job['privileged'] = True
        job['environment']['DRONE_EXTRA_PRIVILEGED'] = 'True'

    elif special == 'ubsan':
        job['name'] = 'UBSan ' + job['name']
        if job['compiler'] == 'gcc':
            set_or_append(job['environment'], 'B2_LINKFLAGS', '-fuse-ld=gold')

    elif special == 'tsan':
        job['name'] = 'TSan ' + job['name']

    elif special == 'valgrind':
        job['name'] = 'Valgrind ' + job['name']
        job['type'] = 'valgrind'
        set_or_append(
            job['environment'], 'B2_TESTFLAGS', 'testing.launcher=valgrind',
        )
        job['environment']['VALGRIND_OPTS'] = '--error-exitcode=1'

        if job['compiler'] == 'clang' and job['os'] == 'linux':
            job.setdefault('packages', []).append('libc6-dbg')

    if special in ('asan', 'tsan', 'ubsan'):
        job['environment']['B2_' + special.upper()] = '1'

        if job['compiler'] == 'clang' and job['os'] == 'linux':
            job.setdefault('packages', []).append(
                'libclang-rt-%s-dev' % job['version']
            )
    if special in ('asan', 'tsan', 'ubsan', 'valgrind'):
        job['environment']['B2_VARIANT'] = 'debug'
        job['environment']['COMMENT'] = special
        set_or_append(
            job['environment'], 'B2_DEFINES', 'BOOST_NO_STRESS_TEST=1',
        )

_supported_compilers = {
    'gcc': {
        'versions': ['14', '13', '12', '11', '10', '9', '8.3', '8', '7', '6', '5', '4.9', '4.8'],
        'display_name': 'GCC',
        'executable': 'g++',
        '14': {
            'latest_cpp': 2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['g++-14'],
                }
            },
        },
        '13': {
            'latest_cpp': 2026,
            'custom_cpp': { 2026: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['g++-13'],
                }
            },
        },
        '12': {
            'latest_cpp': 2026,
            'custom_cpp': { 2026: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['g++-12'],
                },
                ('linux', 'arm64'): {
                    'image': 'cppalliance/droneubuntu2204:multiarch',
                    'packages': ['g++-12'],
                },
                ('linux', 's390x'): {
                    'image': 'cppalliance/droneubuntu2204:multiarch',
                    'packages': ['g++-12'],
                },
            },
        },
        '11': {
            'latest_cpp': 2026,
            'custom_cpp': { 2026: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2004:1',
                    'packages': ['g++-11'],
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                    'environment': {
                        'B2_LINKFLAGS': '-Wl,-rpath=/usr/local/lib/gcc11',
                    },
                },
                ('linux', 'arm64'): {
                    'image': 'cppalliance/droneubuntu2004:multiarch',
                    'packages': ['g++-11'],
                },
                ('linux', 's390x'): {
                    'image': 'cppalliance/droneubuntu2004:multiarch',
                    'packages': ['g++-11'],
                },
            },
        },
        '10': {
            'latest_cpp': 2020,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2004:1',
                    'packages': ['g++-10'],
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '9': {
            'latest_cpp': 2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['g++-9'],
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        # Explicitly testing 8.3 is useful since gcc >=8 <8.4 contains a bug
        # regarding ambiguity in implicit conversions to
        # boost::core::string_view in C++17 mode.
        '8.3': {
            'latest_cpp': 2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:gcc-8.3.0',
                    'environment': { 'B2_TOOLSET': 'gcc-8' },
                },
            },
        },
        '8': {
            'latest_cpp': 2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['g++-8'],
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '7': {
            'latest_cpp': 2017,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['g++-7'],
                },
            },
        },
        '6': {
            'latest_cpp': 2017,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['g++-6'],
                },
            },
        },
        '5': {
            'latest_cpp': 2017,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['g++-5'],
                },
            },
        },
        '4.9': {
            'latest_cpp': 2014,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1604:1',
                    'packages': ['g++-4.9'],
                },
            },
        },
        '4.8': {
            'latest_cpp': 2014,
            'custom_cpp': { 2014: '1y' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1604:1',
                    'packages': ['g++-4.8'],
                },
            },
        },
    },
    'clang': {
        'versions': ['23', '22', '21', '20', '19', '18', '17', '16', '15', '14', '13', '12', '11', '10', '9', '8', '7', '6.0', '5.0', '4.0', '3.8'],
        'display_name': 'Clang',
        'executable': 'clang++',
        '23': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-23'],
                    'llvm_os': 'noble',
                },
            },
        },
        '22': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-22'],
                    'llvm_os': 'noble',
                    'llvm_ver': '22',
                },
            },
        },
        '21': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-21'],
                    'llvm_os': 'noble',
                    'llvm_ver': '21',
                },
            },
        },
        '20': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-20'],
                    'llvm_os': 'noble',
                    'llvm_ver': '20',
                },
            },
        },
        '19': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-19'],
                    'llvm_os': 'noble',
                    'llvm_ver': '19',
                },
            },
        },
        '18': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-18'],
                    'llvm_os': 'noble',
                    'llvm_ver': '18',
                },
            },
        },
        '17': {
            'latest_cpp':  2026,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2404:1',
                    'packages': ['clang-17'],
                    'llvm_os': 'noble',
                    'llvm_ver': '17',
                },
            },
        },
        '16': {
            'latest_cpp':  2023,
            'custom_cpp': { 2023: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-16'],
                    'llvm_os': 'jammy',
                    'llvm_ver': '16',
                },
            },
        },
        '15': {
            'latest_cpp':  2023,
            'custom_cpp': { 2023: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-15',  'libstdc++-10-dev'],
                    'llvm_os': 'jammy',
                    'llvm_ver': '15',
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '14': {
            'latest_cpp':  2023,
            'custom_cpp': { 2023: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-14', 'libstdc++-10-dev'],
                    'llvm_os': 'jammy',
                    'llvm_ver': "14",
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
                ('linux', 's390x'): {
                    'image': 'cppalliance/droneubuntu2204:multiarch',
                    'packages': ['clang-14', 'libstdc++-10-dev'],
                    'llvm_os': 'jammy',
                    'llvm_ver': "14",
                },
            },
        },
        '13': {
            'latest_cpp':  2020,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-13', 'libstdc++-10-dev'],
                    'llvm_os': 'jammy',
                    'llvm_ver': '13',
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '12': {
            'latest_cpp':  2023,
            'custom_cpp': { 2023: '2b' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-12', 'libstdc++-9-dev'],
                    'llvm_os': 'focal',
                    'llvm_ver': '12',
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
                ('linux', 'arm64'): {
                    'image': 'cppalliance/droneubuntu2004:multiarch',
                    'packages': ['clang-12', 'libstdc++-9-dev'],
                    'llvm_os': 'focal',
                    'llvm_ver': '12',
                },
            },
        },
        '11': {
            'latest_cpp':  2020,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu2204:1',
                    'packages': ['clang-11', 'libstdc++-9-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '11',
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '10': {
            'latest_cpp':  2020,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-10', 'libstdc++-9-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '10',
                },
                ('freebsd', 'x86_64'): {
                    'os_version': '13.1',
                },
            },
        },
        '9': {
            'latest_cpp':  2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-9', 'libstdc++-9-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '9',
                },
            },
        },
        '8': {
            'latest_cpp':  2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-8', 'libstdc++-8-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '8',
                },
            },
        },
        '7': {
            'latest_cpp':  2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-7', 'libstdc++-8-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '7',
                },
            },
        },
        '6.0': {
            'latest_cpp':  2020,
            'custom_cpp': { 2020: '2a' },
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-6.0', 'libc6-dbg', 'libstdc++-8-dev'],
                    'llvm_os': 'bionic',
                    'llvm_ver': '6.0',
                },
            },
        },
        '5.0': {
            'latest_cpp':  2014,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-5.0'],
                    'environment': {'UBUNTU_TOOLCHAIN_DISABLE': 'true'}
                },
            },
        },
        '4.0': {
            'latest_cpp':  2014,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1804:1',
                    'packages': ['clang-4.0', 'libstdc++-6-dev'],
                    'llvm_os': 'xenial',
                    'llvm_ver': '4.0',
                },
            },
        },
        '3.8': {
            'latest_cpp':  2014,
            'platforms': {
                ('linux', 'x86_64'): {
                    'image': 'cppalliance/droneubuntu1604:1',
                    'packages': ['clang-3.8', 'g++'],
                    'llvm_os': 'xenial',
                    'llvm_ver': '3.8',
                },
            },
        },
    },
    'msvc': {
        'versions': ['14.5', '14.3', '14.2', '14.1', '14.0'],
        'display_name': 'MSVC',
        '14.5': {
            'latest_cpp': 2023,
            'custom_cpp': { 2023: 'latest' },
            'platforms': {
                ('windows', 'x86_64'): {
                    'image': 'cppalliance/dronevs2026:1',
                }
            },
        },
        '14.3': {
            'latest_cpp': 2020,
            'platforms': {
                ('windows', 'x86_64'): {
                    'image': 'cppalliance/dronevs2022:1',
                }
            },
        },
        '14.2': {
            'latest_cpp': 2020,
            'custom_cpp': { 2020: 'latest' },
            'platforms': {
                ('windows', 'x86_64'): {
                    'image': 'cppalliance/dronevs2019:2',
                }
            },
        },
        '14.1': {
            'latest_cpp': 2017,
            'platforms': {
                ('windows', 'x86_64'): {
                    'image': 'cppalliance/dronevs2017',
                },
            },
        },
        '14.0': {
            'latest_cpp': 2014,
            'platforms': {
                ('windows', 'x86_64'): {
                    'image': 'cppalliance/dronevs2015',
                    'environment': {"B2_DONT_EMBED_MANIFEST": 1},
                },
            },
        },
    },
    'apple-clang': {
        'versions': ['16.2.0', '15.4.0', '13.4.1'],
        'display_name': 'Apple-Clang',
        '16.2.0': {
            'latest_cpp': 2017,
            'platforms': {
                ('macos', 'arm64'): {
                    'xcode_version': '16.2.0',
                    'environment': { 'B2_TOOLSET': 'clang' },
                },
            },
        },
        '15.4.0': {
            'latest_cpp': 2017,
            'platforms': {
                ('macos', 'arm64'): {
                    'xcode_version': '15.4.0',
                    'environment': { 'B2_TOOLSET': 'clang' },
                },
            },
        },
        '13.4.1': {
            'latest_cpp': 2017,
            'platforms': {
                ('macos', 'arm64'): {
                    'xcode_version': '13.4.1',
                    'environment': { 'B2_TOOLSET': 'clang' },
                },
            },
        },
    },
}

# from https://github.com/cppalliance/ci-automation
load('@ci_automation//ci/drone/:functions.star', 'version_match', 'parse_semver_range', 'parse_semver', 'linux_cxx', 'windows_cxx', 'osx_cxx', 'freebsd_cxx')
