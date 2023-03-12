#include "drip.h"

#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <unordered_set>

#include <poll.h>

#ifndef HEADER
#define HEADER '-'
#endif

using namespace std;

struct drip
{
  char id;
  stringstream stdin;
  stringstream stdout;
  stringstream stderr;
};

namespace
{
  static const size_t _SUCCESS = 0;
  static const size_t _FAILURE = 1;
  unordered_set<drip_t> instances;
  drip_t find_instance(char id)
  {
    for (auto instance : instances)
      if (instance->id == id)
        return instance;
    return nullptr;
  }
  bool has_instance(drip_t instance)
  {
    return instance && find_instance(instance->id) != nullptr;
  }
}

drip_t drip_alloc()
{
  auto instance = new drip();
  char id = (char)instances.size() + '0';
  instance->id = id;
  instances.insert(instance);
  return instance;
}

size_t drip_close(drip_t instance)
{
  if (!has_instance(instance))
    return _FAILURE;
  instances.erase(instance);
  delete instance;
  return _SUCCESS;
}

size_t drip_fetch(drip_t instance, char *msg, size_t length)
{
  if (!has_instance(instance))
    return _FAILURE;
  auto stdin_length = instance->stdin.str().size();
  if (stdin_length == 0)
    return 0;
  instance->stdin.read(msg, length);
  return instance->stdin.gcount();
}

size_t drip_print(drip_t instance, const char *msg, size_t length)
{
  if (!has_instance(instance))
    return _FAILURE;
  stringstream ss;
  ss << HEADER << HEADER << instance->id << hex << setfill('0') << setw(8) << length << msg << '\n';
  instance->stdout << ss.str();
  return _SUCCESS;
}

size_t drip_error(drip_t instance, const char *err, size_t length)
{
  if (!has_instance(instance))
    return _FAILURE;
  stringstream ss;
  ss << HEADER << HEADER << instance->id << hex << setfill('0') << setw(8) << length << err << '\n';
  instance->stderr << ss.str();
  return _SUCCESS;
}

size_t drip_dirty()
{
  bool has_output = false;
  for (auto instance : instances)
  {
    const auto &out = instance->stdout.str();
    if (!out.empty())
      has_output = true;
    const auto &err = instance->stderr.str();
    if (!err.empty())
      has_output = true;
  }
  if (has_output)
    return _FAILURE;
  bool has_input = true;
  struct pollfd stdin_poll = {.fd = 0, .events = POLLIN};
  if (poll(&stdin_poll, 1, 0) <= 0)
    has_input = false;
  if (has_input)
    return _FAILURE;
  return _SUCCESS;
}

size_t drip_cycle()
{
  for (auto instance : instances)
  {
    const auto &out = instance->stdout.str();
    if (!out.empty())
    {
      cout << out;
      instance->stdout.str("");
      cout.flush();
    }
    const auto &err = instance->stderr.str();
    if (!err.empty())
    {
      cerr << err;
      instance->stderr.str("");
      cerr.flush();
    }
  }

  struct pollfd stdin_poll = {.fd = 0, .events = POLLIN};
  if (poll(&stdin_poll, 1, 0) <= 0)
    return _SUCCESS;

  string line;
  getline(cin, line);
  auto rewind = [=]()
  {
    for (auto it = line.rbegin(); it != line.rend(); ++it)
      cin.putback(*it);
    return _FAILURE;
  };

  if (line.size() < 4 || line[0] != HEADER || line[1] != HEADER)
    return rewind();

  char id = line[2];
  drip_t instance = find_instance(id);
  if (!instance)
    return rewind();

  const auto &
      length_str = line.substr(3, 8);
  for (auto c : length_str)
    if (!isxdigit(c))
      return rewind();
  size_t length = stoul(line.substr(3, 8), nullptr, 16);
  if (line.size() < 11 + length)
    return rewind();

  const auto &msg = line.substr(11, length);
  instance->stdin << msg << '\n';

  return _SUCCESS;
}
