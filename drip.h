#ifndef DRIP_H
#define DRIP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct drip *drip_t;

  /** @brief Allocate a new instance of the drip library */
  drip_t drip_alloc();
  /** @brief Close an instance of the drip library */
  size_t drip_close(drip_t instance);
  /** @brief Equivalent to read() from stdin */
  size_t drip_fetch(drip_t instance, char *msg, size_t length);
  /** @brief Equivalent to write() on stdout */
  size_t drip_print(drip_t instance, const char *msg, size_t length);
  /** @brief Equivalent to write() on stderr */
  size_t drip_error(drip_t instance, const char *err, size_t length);
  /** @brief checks if to see cycle needs to be called */
  size_t drip_dirty();
  /** @brief cycles the stdin, stdout and stderr streams */
  size_t drip_cycle();

#ifdef __cplusplus
}

#include <string>
#include <memory>

/** @brief C++ library for interleaved IPC messaging through stdio with a C ABI */
class Drip : public std::enable_shared_from_this<Drip>
{
  drip_t m_instance;
  explicit Drip() : m_instance(nullptr) { m_instance = drip_alloc(); }

public:
  static std::shared_ptr<Drip> alloc() { return std::shared_ptr<Drip>(new Drip()); }
  size_t fetch(char *msg, size_t length) { return drip_fetch(m_instance, msg, length); }
  size_t fetch(std::string &msg) { return drip_fetch(m_instance, &msg[0], msg.size()); }
  size_t print(const char *msg, size_t length) { return drip_print(m_instance, msg, length); }
  size_t print(const std::string &msg) { return drip_print(m_instance, msg.c_str(), msg.size()); }
  size_t error(const char *err, size_t length) { return drip_error(m_instance, err, length); }
  size_t error(const std::string &err) { return drip_error(m_instance, err.c_str(), err.size()); }
  static size_t dirty() { return drip_dirty(); }
  static size_t cycle() { return drip_cycle(); }
  ~Drip()
  {
    drip_close(m_instance);
    m_instance = nullptr;
  }
};

#endif

#endif
