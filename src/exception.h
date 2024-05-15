#pragma once

#include <exception>

class Close : public std::exception {};
class Bad_format : public std::exception {};
