#### PGJson介绍
* PGJson是一个使用C++11编写的Json解析器，现阶段只提供DOM风格解析。
    * 使用C++11编写，类型架构分明，代码风格良好。
    * 只使用了STL的极小部分（只使用了std::vector做解析时的buffer，还有可通过宏开关关闭使用的对外std::string接口），不依赖任何第三方库、系统API和编译器扩展，使用CMake管理项目，跨平台跨编译器。
    * 尽量在降低头文件污染和inline提速上寻找平衡。
    * 使用时链接一个静态库即可，使用方便。
    * 使用高效的对象/内存池分配管理Json对象。
    * 优化基础组件提高效率，减少不必要的复制开销。
    * 支持标准Json并支持一部分扩展功能。
    * 解析、格式化Json均使用输入输出流，支持定制扩展。
    * 提供对用户使用体验良好的API并且同时不完全封闭内部函数，方便用户实现更精细的扩展。
    * 利用C++11变参模板做了更舒服的API(还未完全完成)。
    * 良好的ParseError的处理(未实现，会加入错误栈等更精细的错误信息处理)
    * 基于对象的编程风格，未使用虚函数，提高效率。
    * 使用RAII管理对象，并且使用RAII管理管理对象的组件，降低资源泄露的风险同时使管理自动化，降低耦合。
    * 解析和字符串处理均使用字节流而非字符流，为以后增加编码支持提供基础。
    * 未来会实现更多的扩展和对内存和错误处理的更精细的控制。
    * 未来会实现大数和高精度数的存储和处理。
    * 未来会提供UTF8、GBK、Unicode支持。
* PGJson仓库 ： [PGJson·Gitee](https://gitee.com/pgzxb/pgjson)
#### PGJson安装
* 由于本项目使用CMake管理，安装简便。
* clone本仓库到本地
```shell script
git clone https://gitee.com/pgzxb/pgjson.git
```
* 进入pgjson目录，创建build目录并进入
```shell script
cd pgjson
mkdir build
cd build
```
* 执行cmake
```shell script
cmake ..
```
* 生成
    * 如果是MSVC就打开ALL_BUILD项目生成即可（注意如果MSVC生成Release版将无法使用断言）
    * 如果是MinGW或MacOS/Linux，执行```mingw32-make```或```make```
* 生成静态库文件链接使用即可。

* MinGW64下的全过程
```shell script
git clone https://gitee.com/pgzxb/pgjson.git
cd pgjson
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
# 最终在pgjson/build中有libPGJson.a静态库文件
```
* 最终生成PGJson和PGTest两个项目，PGTest现无实质内容。
#### Hello PGJson
* PGJson入门示例
```C++
#include <iostream>
#include <PGJson/Document.h>

using namespace pg::base::json;

int main() {
    // Only when PGJSON_DEBUG was defined, can assertion be used in PGJson.
    // Plenty of assertions are used in PGJson to DEBUG.
    // Recommend you define the macro.

    const char json[] = "{\n"
                        "   \"From\" : {\"name\" : \"PGZXB\", \"age\" : 19, \"GPA\" : 4.14},\n"
                        "   \"To\" : \"PGJson\",\n"
                        "   \"sites\": [\n"
                        "       { \"name\":\"CSDN\" , \"url\": \"https://blog.csdn.net/PGZXB?spm=1011.2124.3001.5343\" },\n"
                        "       { \"name\":\"Gitee\" , \"url\":\"https://gitee.com/pgzxb\" },\n"
                        "       { \"name\":\"Zhihu\" , \"url\":\"https://www.zhihu.com/people/geek-81-44\" },\n"
                        "   ]\n"
                        "}\n";  // The Json-String that will be parsed
                        
    // Create Document, which can parse from string/file/stream and stringify DOM to file/string/stream
    Document document;

    document.parseFromString(json);  // parse json form string

    Node & DOM = document.d();  // get DOM-Root
    
    // add "msg" : "Hello PGJson!" to DOM-Tree-Root-Object
    DOM.addMember("msg")->value.setString("Hello PGJson!");
    
    // stringify the json-tree, default : format-open
    std::cout << document.stringify() << "\n\n";
    
    // stringify the json-tree, format-close
    std::cout << document.stringify(false) << "\n\n";
    return 0;
}
```
* g++编译
```shell script
# 在pgjson目录下
g++ hello_pgjson.cpp -L build -lPGJson -o hello_pgjson -I include -DPGJSON_DEBUG -Wall
```
* 运行结果
```shell script
{
        "From" : {
                "name" : "PGZXB",
                "age" : 19,
                "GPA" : 4.140000
        },
        "To" : "PGJson",
        "sites" : [
                {
                        "name" : "CSDN",
                        "url" : "https://blog.csdn.net/PGZXB?spm=1011.2124.3001.5343"
                },
                {
                        "name" : "Gitee",
                        "url" : "https://gitee.com/pgzxb"
                },
                {
                        "name" : "Zhihu",
                        "url" : "https://www.zhihu.com/people/geek-81-44"
                }
        ],
        "msg" : "Hello PGJson!"
}

{"From" : {"name" : "PGZXB","age" : 19,"GPA" : 4.140000},"To" : "PGJson","sites" : [{"name" : "CSDN","url" : "https://blog.csdn.net/PGZXB?spm=1011.2124.
3001.5343"},{"name" : "Gitee","url" : "https://gitee.com/pgzxb"},{"name" : "Zhihu","url" : "https://www.zhihu.com/people/geek-81-44"}],"msg" : "Hello PG
Json!"}


```
#### 现支持的扩展
* 单行注释```//```
* Object最后一个字段后可加```,```
* Array最后一个Item后可加```,```

#### 更多功能、代码示例期待后续---