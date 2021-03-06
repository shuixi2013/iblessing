//
//  main.m
//  iblessing
//
//  Created by soulghost on 2020/7/20.
//  Copyright © 2020 soulghost. All rights reserved.
//

#include <cstdio>
#include "util/StringUtils.h"
#include "vendor/argparse/argparse.h"
#include "vendor/termcolor/termcolor.h"
#include "scanner/ScannerDispatcher.hpp"
#include "generator/GeneratorDispatcher.hpp"
#include "platform/macos/csrutil.hpp"

using namespace std;
using namespace argparse;
using namespace iblessing;

int main(int argc, const char *argv[]) {
    // ascii art
    printf("\n\
           ☠️\n\
           ██╗██████╗ ██╗     ███████╗███████╗███████╗██╗███╗   ██╗ ██████╗\n\
           ██║██╔══██╗██║     ██╔════╝██╔════╝██╔════╝██║████╗  ██║██╔════╝\n\
           ██║██████╔╝██║     █████╗  ███████╗███████╗██║██╔██╗ ██║██║  ███╗\n\
           ██║██╔══██╗██║     ██╔══╝  ╚════██║╚════██║██║██║╚██╗██║██║   ██║\n\
           ██║██████╔╝███████╗███████╗███████║███████║██║██║ ╚████║╚██████╔╝\n\
           ╚═╝╚═════╝ ╚══════╝╚══════╝╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝\n\
           \n");
    
    // hello text
    printf("[***] iblessing iOS Security Exploiting Toolkit Beta 0.2.0.8x (http://blog.asm.im)\n");
    printf("[***] Author: Soulghost (高级页面仔) @ (https://github.com/Soulghost)\n");
    if (CSRUtil::isSIPon()) {
        printf("[***] System Integrity Protection is on\n");
    }
    printf("\n");
    
    // parse args
    ArgumentParser parser("iblessing", "iblessing iOS security toolkit");
    parser.add_argument()
    .names({"-m", "--mode"})
    .description("mode selection:\n\
                                * scan: use scanner\n\
                                * generator: use generator");
    
    parser.add_argument()
    .names({"-i", "--identifier"})
    .description("choose module by identifier:\n\
                                * <scanner-id>: use specific scanner\n\
                                * <generator-id>: use specific generator");
    
    parser.add_argument()
    .names({"-f", "--file"})
    .description("input file path");
    
    parser.add_argument()
    .names({"-o", "--output"})
    .description("output file path");
    
    parser.add_argument()
    .names({"-l", "--list"})
    .description("list available scanners");
    
    parser.add_argument()
    .names({"-d", "--data"})
    .description("extra data");
    
    parser.enable_help();
    
    // hanle parse error
    auto err = parser.parse(argc, argv);
    if (err) {
        parser.print_help();
        return 1;
    }
    
    // handle help
    if (argc == 1 || parser.exists("help")) {
        parser.print_help();
        return 1;
    }
    
    // handle scanner list
    if (parser.exists("list")) {
        ScannerDispatcher *sd = new ScannerDispatcher();
        vector<Scanner *> scanners = sd->allScanners();
        printf("[*] Scanner List:\n");
        for (Scanner *scanner : scanners) {
            printf("    - %s: %s\n", scanner->identifier.c_str(), scanner->desc.c_str());
            delete scanner;
        }
        delete sd;
        
        GeneratorDispatcher *gd = new GeneratorDispatcher();
        vector<Generator *> generators = gd->allGenerators();
        printf("\n[*] Generator List:\n");
        for (Generator *generator : generators) {
            printf("    - %s: %s\n", generator->identifier.c_str(), generator->desc.c_str());
            delete generator;
        }
        delete gd;
        return 0;
    }
    
    // handle info mode
    string mode = parser.get<string>("mode");
    if (mode == "generator") {
        if (!parser.exists("file")) {
            cout << termcolor::red;
            cout << "[-] Error: please use -f to set the input file";
            cout << termcolor::reset << endl;
            return 1;
        }
        
        string outputFilePath;
        if (parser.exists("output")) {
            outputFilePath = parser.get<string>("output");
        } else {
            size_t size = pathconf(".", _PC_PATH_MAX);
            char *buf = (char *)malloc((size_t)size);
            char *path = getcwd(buf, (size_t)size);
            outputFilePath = string(path);
            free(buf);
        }
        printf("[*] set output path to %s\n", outputFilePath.c_str());
        
        string generatorId;
        if (!parser.exists("identifier")) {
            cout << termcolor::red;
            cout << "[-] Error: please use -i to set the generator by id";
            cout << termcolor::reset << endl;
            return 1;
        }
        generatorId = parser.get<string>("identifier");
        
        map<string, string> options;
        if (parser.exists("data")) {
            string extraData = parser.get<string>("data");
            vector<string> ops = StringUtils::split(extraData, ';');
            for (string op : ops) {
                vector<string> lr = StringUtils::split(op, '=');
                if (lr.size() != 2) {
                    cout << termcolor::red;
                    cout << "[-] Error: cannot parse extra data " << op;
                    cout << termcolor::reset << endl;
                    return 1;
                }
                options[lr[0]] = lr[1];
            }
        }
        
        string filePath = parser.get<string>("file");
        printf("[*] input file is %s\n", filePath.c_str());
        GeneratorDispatcher *generator = new GeneratorDispatcher();
        int ret = generator->start(generatorId, options, filePath, outputFilePath);
        delete generator;
        return ret;
    } else if (mode == "scan") {
        if (!parser.exists("file")) {
            cout << termcolor::red;
            cout << "[-] Error: please use -f to set the input file";
            cout << termcolor::reset << endl;
            return 1;
        }
        
        string outputFilePath;
        if (parser.exists("output")) {
            outputFilePath = parser.get<string>("output");
        } else {
            size_t size = pathconf(".", _PC_PATH_MAX);
            char *buf = (char *)malloc((size_t)size);
            char *path = getcwd(buf, (size_t)size);
            if (path) {
                outputFilePath = string(path);
            } else {
                outputFilePath = "/tmp";
            }
            free(buf);
        }
        printf("[*] set output path to %s\n", outputFilePath.c_str());
        
        string scannerId;
        if (!parser.exists("identifier")) {
            cout << termcolor::red;
            cout << "[-] Error: please use -i to set the scanner by id";
            cout << termcolor::reset << endl;
            return 1;
        }
        scannerId = parser.get<string>("identifier");
        
        map<string, string> options;
        if (parser.exists("data")) {
            string extraData = parser.get<string>("data");
            vector<string> ops = StringUtils::split(extraData, ';');
            for (string op : ops) {
                vector<string> lr = StringUtils::split(op, '=');
                if (lr.size() != 2) {
                    cout << termcolor::red;
                    cout << "[-] Error: cannot parse extra data " << op;
                    cout << termcolor::reset << endl;
                    return 1;
                }
                options[lr[0]] = lr[1];
            }
        }
        
        string filePath = parser.get<string>("file");
        printf("[*] input file is %s\n", filePath.c_str());
        ScannerDispatcher *scanner = new ScannerDispatcher();
        int ret = scanner->start(scannerId, options, filePath, outputFilePath);
        delete scanner;
        return ret;
    } else {
        cout << termcolor::red;
        cout << "[-] error: unresolved mode: " << mode;
        cout << termcolor::reset << endl;
    }
    return 0;
}
