#include "WrapperGenerator.h"

using namespace std;

WrapperGenerator::WrapperGenerator(shared_ptr<unordered_map<string, 
                                   FunctionPrototype>> _prototypeMap,
                                   string pathPrefix=""):
                                   prototypeMap(_prototypeMap) {
    headerFile.open(pathPrefix + "VProfEventWrappers.h");
    implementationFile.open(pathPrefix + "VProfEventWrappers.cpp");

    const string generatedFileMessage = 
        " ///////////////////////////////////////////////////// "
        " // Note that this file was generated by VProfiler. // "
        " // Please do not change the contents of this file! // "
        " ///////////////////////////////////////////////////// \n\n";

    headerFile << generatedFileMessage; 
    implementationFile << generatedFileMessage;
}

void WrapperGenerator::GenerateHeader() {
    headerFile << "#include \"TraceTool.h\"\n\n";

    for (auto kv : *prototypeMap) {
        headerFile << kv.second.functionPrototype + ";\n\n";
    }

    headerFile.close();
}

void WrapperGenerator::GenerateImplementations() {
    implementationFile << "#include \"VProfEventWrappers.h\"\n\n";

    for (auto kv : *prototypeMap) {
        if (kv.second.returnType != "void") {
            implementationFile << kv.second.returnType + " result;\n";
        }

        implementationFile << kv.second.functionPrototype + " {\n";
        implementationFile << "EventTraceTool::LogEventStart(" + 
                              to_string(logInfoMap[kv.first].functionID) + ", " 
                              + (logInfoMap[kv.first].isMessageBased ? "true" : "false")  
                              + ");\n";

        if (kv.second.returnType != "void") {
            implementationFile << "result = ";
        }
        implementationFile << kv.second.innerCallPrefix + "(";

        for (int i = 0, j = kv.second.paramVars.size(); i < j; i++) {
            implementationFile << kv.second.paramVars[i];

            if (i != (j - 1)) {
                implementationFile << ", ";
            }
        }

        implementationFile <<");\n";

        implementationFile << "EventTraceTool::LogEventEnd(" + 
                              to_string(logInfoMap[kv.first].functionID) + ", " 
                              + (logInfoMap[kv.first].isMessageBased ? "true" : "false")  
                              + ");\n";

        if (kv.second.returnType != "void") {
            implementationFile << "return result;\n";
        }

        implementationFile << "}\n\n";
    }

    implementationFile.close();
}

void WrapperGenerator::GenerateWrappers() {
    GenerateHeader();
    GenerateImplementations();
}
