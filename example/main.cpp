#include <chrono>
#include <iostream>
#include <string>

#include <luasel/Luasel.hpp>

#define STRINGIFY(...) #__VA_ARGS__

void f(Luasel::CallHelper& call)
{
    auto const& args = call.GetArgList();
    auto it = args.begin();
    auto itEnd = args.end();
    for (; it != itEnd; ++it)
        std::cout << "arg: " << it->ToNumber() << std::endl;
    std::cout << call.PopArg().CheckBoolean() << std::endl;
    std::cout << call.PopArg().CheckString() << std::endl;
    std::cout << "f()" << std::endl;
    call.PushRet(call.GetInterpreter().MakeNumber(1.5));
    call.PushRet(call.GetInterpreter().MakeNumber(2.6));
}

void TestFunction(int test)
{
    std::cout << test << std::endl;
}

static void Basic(Luasel::Interpreter& i)
{
    i.DoString("test = {} test[4] = 23.34");

    //Luasel::Ref bite = i.MakeFunction(i.Bind(&f, std::placeholders::_1);
    Luasel::Ref bite = i.MakeFunction(std::bind(&f, std::placeholders::_1));

    try
    {
        std::cout << bite(1, 2).ToNumber() << " jksdfgjk " << sizeof(Luasel::Ref) << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "from c++: " << e.what() << std::endl;
    }

    i.Globals().Set(std::string("bite"), bite);

    try
    {
        i.DoString("bite(1, 2)");
    }
    catch (std::exception& e)
    {
        std::cout << "from lua: " << e.what() << std::endl;
    }

    {
        Luasel::CallHelper helper(i);
        helper.PushArg(i.Make(true));
        helper.PushArg(i.Make("Hey!!"));
        bite.Call(helper);
        auto const& rets = helper.GetRetList();
        auto it = rets.begin();
        auto itEnd = rets.end();
        for (; it != itEnd; ++it)
            std::cout << "ret: " << it->ToString() << std::endl;
        i.DoString("ret1, ret2 = bite(false, \"ZOB\")");
        std::cout << "ret1: " << i.Globals()["ret1"].ToString() << ", ret2: " << i.Globals()["ret2"].ToString() << std::endl;
    }
}

static void Iterators(Luasel::Interpreter& i)
{
    i.DoString("test = \"sjkldg\\nlkéhsdfg\" g = 323.4 jdsk = {} iwer = {} jkd = function() end");

    Luasel::Iterator it = i.Globals().Begin();
    Luasel::Iterator itEnd = i.Globals().End();
    for (; it != itEnd; ++it)
    {
        std::cout << it.GetValue().GetTypeName() << ": " << it.GetValue().ToString() << " (key " << it.GetKey().GetTypeName() << " " << it.GetKey().ToString() << ")" << std::endl;
    }
}

static void MetaTables(Luasel::Interpreter& i)
{
    i.DoString("FUCK = function() print \"FUCK\" end");
    Luasel::Ref meta = i.MakeTable();
    meta.Set("__index", i.Globals()["FUCK"]);
    meta.Set(1, "test");
    meta.Set(2, "tests2");
    std::cout << "Type of FUCK: " << meta["__index"].GetTypeName() << std::endl;
    std::cout << "size of metatable: " << meta.GetLength() << std::endl;
    Luasel::Ref metaTest = i.MakeTable();
    metaTest.SetMetaTable(meta);
    i.Globals().Set("metaTest", metaTest);
    i.DoString("a = metaTest[12]");
    i.DoString("b = metaTest[14]");
    i.DoString("c = metaTest[15]");
}

static void Serialization(Luasel::Interpreter& i)
{
    i.DoString("sharedTable = { 213, 23, \"sdfg\" }");
    i.DoString("serializeTest = { bite = { 1, 3, 4, sharedTable }, test = \"HEY\", allo = false }");
    std::string serialized = i.GetSerializer().Serialize(i.Globals()["serializeTest"]);
    std::cout << "serialize1: \"\"\"" << serialized << "\"\"\"\n";
    try
    {
        std::cout << "serialize2: \"\"\"" << i.GetSerializer().Serialize(i.Globals()["FUCK"]) << "\"\"\"\n";
    }
    catch (std::exception& e)
    {
        std::cout << "serialize fail: " << e.what() << std::endl;
    }
    std::cout << "serialize3: \"\"\"" << i.GetSerializer().Serialize(i.Globals()["test"]) << "\"\"\"\n";
    std::cout << "Deserialize: \"\"\"" << i.GetSerializer().Serialize(i.GetSerializer().Deserialize(serialized)) << "\"\"\"\n";

    std::cout << "------- Begin super test de serialization -------" << std::endl;
    auto testObj = i.MakeTable();
    auto testObjMetaTable = i.MakeTable();
    i.DoString("testObjSerialize = function(obj) return \"return nil\" end -- lol");
    testObjMetaTable.Set("__serialize", i.Globals()["testObjSerialize"]);
    testObj.SetMetaTable(testObjMetaTable);
    auto t = i.MakeTable();
    t.Set("h", i.MakeFunction(std::bind(&f, std::placeholders::_1)));
    t.Set("obj", testObj);
    try
    {
        std::cout << i.GetSerializer().Serialize(t, true) << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }
}

static void MetaTableCpp(Luasel::Interpreter& i)
{
    struct A
    {
        int i = 10;
        static A Create()
        {
            return A();
        }
        void Print(int nb) { i = nb; std::cout << "A::Print(" << nb << ")\n"; }
    };


    auto& meta = Luasel::MetaTable::Create(i, A());
    meta.SetMethod("Print", i.Bind(&A::Print));
    meta.SetMetaMethod(Luasel::MetaTable::Collect, i.Bind([](){ std::cout << "collect\n"; }));

    try
    {
        i.DoString("table = getmetatable(m)");
    }
    catch (std::exception& e)
    {
        std::cout << "getmetatable(UserData): " << e.what() << "\n";
    }

    i.Globals().Set("TestFunction", i.Bind(&TestFunction));
    i.DoString("TestFunction(10)");
    //i.Globals().Set("A", i.Bind([](){ return A(); }));
    i.Globals().Set("A", i.Bind(&A::Create));
    i.DoString("a = A()\
               a:Print(50)");

    // VS compiler bug here - construct a new std::string instead of passing it from "Luasel::Ref::Check<std::string>"
    i.Globals().Set("B", i.Bind([](int toto, std::string const& titi) { std::cout << toto << ". " << titi << std::endl; }));
    i.DoString("B('50', 'coucou')");

    i.Globals().Set("C", i.Bind([](double toto, A* titi) { std::cout << toto << ". " << titi->i << std::endl; }));
    i.DoString("C(50, a)");

    std::cout << "first: " << i.GetSerializer().Serialize(i.Globals()["tab"]) << std::endl;
    std::cout << "second: " << i.GetSerializer().Serialize(i.GetSerializer().Deserialize(i.GetSerializer().Serialize(i.Globals()["tab"]))) << std::endl;
}

int main(int, char**)
{
    {
        Luasel::Interpreter i;
        i.RegisterLib(Luasel::LibId::Base);
        i.RegisterLib(Luasel::LibId::Math);

        // Tests basiques (assignation, ...)
        Basic(i);

        // Test itération des globales
        Iterators(i);

        // Test meta-tables lua
        MetaTables(i);

        // Test serialization simple (lua only)
        Serialization(i);

        // Test types C++ (avec serialization)
        MetaTableCpp(i);

    }

    int i = 0;
    Luasel::apply([](std::string const& test, int& i){ std::cout << test << std::endl; i++; }, std::make_tuple("test", std::ref(i)));
    std::cout << i << std::endl;


#ifdef _WIN32
    std::cin.get();
#endif
    return 0;
}
