# Toy Compiler

> This is a rust-like compiler implemented in c++.
>
> 同济大学 2024-2025 学年暑期 **编译原理课程设计** 大作业

这是一个使用 C++ 编写的类 Rust 编译器。项目基于该学年第二学期的 **编译原理** 课程作业，该作业实现了编译器的前端部分，点击[这里](https://github.com/anxi710/compiler-principle-project)查看更多信息。

本项目以模块化、面向对象的方式开发，实现了编译器的前后端，包括：词法分析、语法分析、语义检查、中间代码生成和目标代码。

## Git Convention

Accroding to [Conventional Commits 1.0.0](https://www.conventionalcommits.org/en/v1.0.0/#specification), the commit message should be structured as follows:

> \<type\>[optional scope]: \<description\>
>
> [optional body]
>
> [optional footer(s)]

The commit contains the following structural elements, to communicate intent to the consumers of your library:

1. **fix**: a commit of the *type* `fix` patches a bug in your codebase.

2. **feat**: a commit of the *type* `feat` introduces a new feature to the codebase.

3. **BREAKING CHANGE**: a commit that has a footer `BREAKING CHANGE:`, or appends a `!` after the type/scope, introduces a breaking API change. A BREAKING CHANGE can be part of commits of any *type*.

4. *types* other than `fix:` and `feat:` are allowed, for example: `build:`, `ci:`, `docs:`, `style:`, `refactor:`, `perf:`, `test:`, and others.

5. *footers* other than `BREAKING CHANGE: <description>` may be provided and follow a convention similar to git trailer format.

