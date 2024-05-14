#pragma once

#include <exception>

class Close : public std::exception {};
class BadSaveFormat : public std::exception {};
