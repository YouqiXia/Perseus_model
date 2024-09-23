# Model for Perseus Simulator

<h2> Git Commit Style </h2>

```
[type]: <Subject>
eg: [feat]: new feature or file are added
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
