/**
 * @file OSAL/Common/OSAL.cpp
 * Contains the implementations of common OSAL implementations.
 *
 * @copyright Copyright (C) 2024 Faisal Al-Humaimidi
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
 //==============================================================================

#include <ios>

#include "OSAL.hpp"

AutoAPRPool::AutoAPRPool(apr_pool_t* parent) : pool(nullptr) {
  apr_status_t status = apr_pool_create(&pool, parent);
  if (status != APR_SUCCESS) {
    throw std::runtime_error("Error creating APR pool. Error code: " + std::to_string(status));
  }
}

AutoAPRPool::~AutoAPRPool() {
  if (pool) {
    apr_pool_destroy(pool);
    pool = nullptr;
  }
}

const apr_pool_t* AutoAPRPool::getPool() const {
  return pool;
}

apr_pool_t* AutoAPRPool::getPool() {
  return pool;
}


AutoAPRFile::AutoAPRFile(apr_file_t* file) : file(file) {}

AutoAPRFile::~AutoAPRFile() {
  close();
}

void AutoAPRFile::close() {
  if (file) {
    apr_status_t status = apr_file_close(file);
    if (status != APR_SUCCESS) {
      throw std::runtime_error("Error closing APR file. Error code: " + std::to_string(status));
    }
    file = nullptr;
  }
}

const apr_file_t* AutoAPRFile::getFile() const {
  return file;
}

apr_file_t* AutoAPRFile::getFile() {
  return file;
}

APRFilebuf::APRFilebuf(apr_file_t* file) : file(file) {}

APRFilebuf::int_type APRFilebuf::overflow(int_type c) {
  if (!traits_type::eq_int_type(c, traits_type::eof())) {
    // Sync the buffer here if necessary

    char ch = traits_type::to_char_type(c);
    apr_size_t size = 1;
    if (apr_file_write(file, &ch, &size) == APR_SUCCESS && size == 1) {
      return traits_type::not_eof(c); // Success, not EOF
    }
    else {
      // TODO: Handle error.
      return traits_type::eof();
    }
  }
  // Sync the buffer here if necessary
  // Return a value other than EOF to indicate success when c is EOF
  return traits_type::not_eof(traits_type::eof());
}


APRFilebuf::int_type APRFilebuf::underflow() {
  if (file == nullptr) {
    setg(nullptr, nullptr, nullptr); // Invalidate the buffer
    return traits_type::eof();
  }

  // Attempt to read a single character from the file
  char ch;
  apr_size_t nbytes = 1;
  apr_status_t rv = apr_file_read(file, &ch, &nbytes);

  // Check for EOF or error
  if (rv == APR_EOF || nbytes == 0) {
    setg(nullptr, nullptr, nullptr); // Invalidate the buffer
    return traits_type::eof(); // End of file reached
  }
  else if (rv != APR_SUCCESS) {
    setg(nullptr, nullptr, nullptr); // Invalidate the buffer
    return traits_type::eof(); // Return EOF on error
  }

  // If a character was read successfully, store it in the buffer and return it
  buffer = ch;
  setg(&buffer, &buffer, &buffer + 1); // Set the get pointer to the buffer

  return traits_type::to_int_type(buffer);
}


APRFilebuf::pos_type APRFilebuf::seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) {
  if (file == nullptr) return pos_type(off_type(-1));

  apr_off_t apr_offset = offset; // Convert to apr_off_t
  apr_seek_where_t apr_dir;

  switch (dir) {
  case std::ios_base::beg:
    apr_dir = APR_SET;
    break;
  case std::ios_base::cur:
    apr_dir = APR_CUR;
    break;
  case std::ios_base::end:
    apr_dir = APR_END;
    break;
  default:
    return pos_type(off_type(-1));
  }

  apr_status_t result = apr_file_seek(file, apr_dir, &apr_offset);
  if (result == APR_SUCCESS) {
    return pos_type(apr_offset);
  }
  else {
    return pos_type(off_type(-1));
  }
}

APRFilebuf::pos_type APRFilebuf::seekpos(pos_type pos, std::ios_base::openmode mode) {
  return seekoff(off_type(pos), std::ios_base::beg, mode);
}