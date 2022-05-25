#pragma once

#include <sl/Camera.hpp>

void print(std::string msg_prefix,
           sl::ERROR_CODE err_code = sl::ERROR_CODE::SUCCESS,
           std::string msg_suffix = "");

void parseArgs(int argc, char **argv, sl::InitParameters &param);
