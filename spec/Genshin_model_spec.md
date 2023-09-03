# Model for Genshin Simulator

<h2> Git Commit Style </h2>
```
[type]<Scope>: <Subject>
eg: [feat]<undone>: new feature or file are added
```
| Type | Description |
| :---:  | --- |
|feat	       | A new feature                                                                                            
|fix	       | A bug fix                                                                                               |
|docs	       | Documentation only changes                                                                              |
|style 	     | Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc.) |
|refactor    | A code change that neither fixes a bug nor adds a feature                                               |
|test	       | Adding missing or correcting existing tests                                                             |
|chore	     | Changes to the build process or auxiliary tools and libraries such as documentation generation          |
|perf	       | A code change that improves performance                                                                 |

| Scope  | Description |
| :---:  | --- |
| done   | the change is done        |
| undone | the change is in progress |


<h2> Code Style </h2>
The detail of cpp code style is in <a title="cppguide">https://google.github.io/styleguide/cppguide.html</a>

<h3>the #define Guard </h3>
The format of the symbol name should be <filename>_HH_
```
#ifndef FILENAME_HH_
#define FILENAME_HH_
...
#endif  // FILENAME_HH_
```

## Question
* 疑问
  * writeback(跨pipeline数据传输)
    * writeback的当拍不能使用对应的数据，需要在后面跟一个timebuffer然后写回，有没有更好的做法，怎么做
  * pipeline可以采用递归结构链式索引子pipeline？
    * Scheduler是怎么相互认识彼此的
    * backend那样的结构如何使用scheduler来搭建pipeline，而不是直接写入