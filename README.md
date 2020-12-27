# muon

A subatomic ray tracer.

## Developing

Muon uses [Bazel](https://bazel.build/) as its build system, which you can
[install via these instructions](https://docs.bazel.build/install.html).

To build, run:

```
$ bazel build //muon
```

You can also use [bazel-watcher](https://github.com/bazelbuild/bazel-watcher)
to rebuild automatically after changes.

To build the compilation database, install
[bazel-compilation-database](https://github.com/grailbio/bazel-compilation-database)
and run:

```
$ bazel-compdb -s
```

To run the linter, first build the compilation database and then run:

```
$ clang-tidy muon/*.cc
```
