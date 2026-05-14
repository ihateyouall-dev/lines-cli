![LOGO](docs/lines.svg)

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cad65e1e5e9f4036ba1176238c16a7e4)](https://app.codacy.com/gh/ihateyouall-dev/lines-cli/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![License](https://img.shields.io/github/license/ihateyouall-dev/lines-cli)](LICENSE)
[![Build](https://github.com/ihateyouall-dev/lines-cli/actions/workflows/build.yml/badge.svg)](https://github.com/ihateyouall-dev/lines-cli/actions/workflows/build.yml)
# Lines CLI: command line tool for life planning

lines-cli is a command line task manager.
It provides flexible deadline management, task repetition,
and self-documenting workflows.

### Project status: active development

## Table of contents
- [Lines CLI: command line tool for life planning](#lines-cli-command-line-tool-for-life-planning)
- [Table of contents](#table-of-contents)
- [Features](#features)
- [Building from source](#building-from-source)
- [Installation](#installation)
- [Usage](#usage)
  - [Addition](#addition)
  - [Editing](#editing)
  - [Filtering](#filtering)
  - [Showing](#showing)
  - [Completion](#completion)
  - [Deletion](#deletion)
- [License](#license)

## Features

- task management from command line
- flexible deadline system
- recurring tasks
- tagging system
- rich filtering support

## Building from source

### Requirements:
- C++20 compatible compiler
- CMake 3.23+
- Conan2

To build release version, just run:

```shell
git clone https://github.com/ihateyouall-dev/lines-cli.git
cd lines-cli
make
```

To build debug version with tests, run:

```shell
git clone https://github.com/ihateyouall-dev/lines-cli.git
cd lines-cli
make CMAKE_PRESET=debug-clang CONAN_BUILD_TYPE=Debug
make test
```

## Installation
If you already built project from source, run:
`sudo make install`

Otherwise, see github releases for installers.

## Usage
### Addition
Add task:
`lines-cli tasks add Title`
> NOTE: Task title is required and cannot be empty

Add task with description:
`lines-cli tasks add Title --description Description`

Add task with tags:
`lines-cli tasks add Title --tags Tag1 Tag2 Tag3`

Add task with deadline:
`lines-cli tasks add Title --deadline TimePoint`
See more information about time points in `lines-cli docs timepoints`

Add recurring task:
`lines-cli tasks add Title --repeat RepeatRule`
> NOTE: Repeat rule requires task to have a deadline.
> If no deadline is provided, it will be set to the current time.

Specify deadline together with repeat rule:
`lines-cli tasks add Title --deadline TimePoint --repeat RepeatRule`

Also you can specify time point, when recurrence will end:
`lines-cli tasks add Title --repeat RepeatRule --repeat-end TimePoint`

See more information about time points in `lines-cli docs repeat`

### Editing
Edit task:
`lines-cli tasks edit -i ID --title Title`
Options for 'edit' and 'add' are the same, but 'edit' requires ID of task to edit.

Also 'edit' lets you disable some optional values (like tags, deadline, repeat rule),
just enter 'none' to option you want to disable, tags you can disable by passing empty string:
`lines-cli tasks edit -i ID --deadline none`
`lines-cli tasks edit -i ID --tags ""`

### Filtering
Some of subcommands support filtering tasks to work with.
There's filters you can use and combine at one time:
- `-i,--id` - task with given id.
- `-a,--all` - all tasks.
- `--title REGEX` - tasks whose title fully matches given regex.
- `--title-p REGEX` - tasks whose title partially matches given regex.
- `-T,--any-tag TAGS...` - tasks that have at least one of given tags.
- `-A,--all-tags TAGS...` - tasks that have all of given tags.
- `--ac, --active` - only active tasks.
- `--ex, --expired` - only expired tasks.

### Showing
Show all tasks:
`lines-cli tasks show`

Show filtered tasks:
`lines-cli tasks show [FILTERS]`

### Completion
Complete tasks:
`lines-cli tasks complete [FILTERS]`

Uncomplete tasks:
`lines-cli tasks uncomplete [FILTERS]`

Regular tasks have two states - completed and uncompleted.
Recurrent tasks do not have this states, by completing you advance their deadline.

### Deletion
Delete tasks:
`lines-cli tasks delete [FILTERS]`

## License
Distributed under the LGPL-3.0 License.
See LICENSE for details.
