/**
 * @file Core/Data/VariableStack.cpp
 * Contains the implementation of class Core::Data::VariableStack.
 *
 * @copyright Copyright (C) 2014 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#include "core.h"

namespace Core { namespace Data
{

//==============================================================================
// Static Variables

const Int VariableStack::recordCountIncrement = 100;


//==============================================================================
// Member Functions

void VariableStack::initialize(Word maxStrSize, Word reservedRecordCount, Word reservedLevelCount)
{
  this->release();
  this->maxStrSize = maxStrSize;
  this->reservedRecordCount = reservedRecordCount;
  this->levels.reserve(reservedLevelCount);
  this->buffer = new Byte[this->reservedRecordCount * this->getRecordSize()];
}


void VariableStack::reinitialize(Word maxStrSize, Word reservedRecordCount, Word reservedLevelCount)
{
  if (this->buffer == 0) {
    throw GeneralException(STR("Stack is not initialized."), STR("Core::Data::VariableStack::reinitialize"));
  }
  if (reservedLevelCount > this->levels.capacity()) this->levels.reserve(reservedLevelCount);
  if (this->maxStrSize == maxStrSize && this->reservedRecordCount == reservedRecordCount) return;
  Byte *oldBuf = this->buffer;
  Word oldStrSize = this->maxStrSize;
  this->maxStrSize = maxStrSize;
  this->reservedRecordCount = reservedRecordCount;
  this->buffer = new Byte[this->reservedRecordCount * this->getRecordSize()];

  if (this->isEmpty()) return;
  else if (oldStrSize == this->maxStrSize) {
    memcpy(this->buffer, oldBuf, this->getRecordSize()*this->levels.back());
  } else {
    for (Int i = 0; static_cast<Word>(i) < this->levels.back(); ++i) {
      Byte *destBuf = this->buffer + i*this->getRecordSize();
      Byte *srcBuf = oldBuf + i*VariableStack::getRecordSize(oldStrSize);
      sbstr_cast(destBuf).assign(sbstr_cast(srcBuf), this->maxStrSize);
      *reinterpret_cast<IdentifiableObject**>(destBuf+sizeof(Char)*this->maxStrSize) =
          *reinterpret_cast<IdentifiableObject**>(srcBuf+sizeof(Char)*oldStrSize);
    }
  }
  delete[] oldBuf;
}


void VariableStack::copy(const VariableStack *src)
{
  if (src->buffer == 0) {
    throw InvalidArgumentException(STR("src"), STR("Core::Data::VariableStack::copy"),
                                   STR("Stack is not initialized."));
  }

  // If we have a small buffer, we can drop it.
  if (this->buffer!=0 && (this->maxStrSize < src->maxStrSize ||
                          this->reservedRecordCount < src->reservedRecordCount)) {
    delete[] this->buffer;
    this->buffer = 0;
  }

  // If we don't have a buffer, we need to create one with the same size.
  if (this->buffer == 0) {
    this->maxStrSize = src->maxStrSize;
    this->reservedRecordCount = src->reservedRecordCount;
    this->buffer = new Byte[this->reservedRecordCount * this->getRecordSize()];
  }

  this->levels.reserve(src->levels.capacity());

  if (this->maxStrSize == src->maxStrSize && this->reservedRecordCount == src->reservedRecordCount) {
    // Copy the whole buffer at once.
    memcpy(this->buffer, src->buffer, this->getRecordSize()*src->levels.back());
    this->levels = src->levels;
    this->trunkStack = src->trunkStack;
    this->trunkLevelIndex = src->trunkLevelIndex;
  } else {
    // Copy the levels one by one.
    this->clear();
    this->trunkStack = src->trunkStack;
    this->trunkLevelIndex = src->trunkLevelIndex;
    for (Int i = 0; static_cast<Word>(i) < src->levels.size(); ++i) {
      this->copyLevel(src, this->trunkLevelIndex + 1 + i);
    }
  }
}


void VariableStack::copyLevel(const VariableStack *src, Int level)
{
  if (this->buffer == 0) {
    throw GeneralException(STR("Stack is not initialized"), STR("Core::Data::VariableStack::copyLevel"));
  }
  if (src->buffer == 0) {
    throw InvalidArgumentException(STR("src"), STR("Core::Data::VariableStack::copyLevel"),
                                   STR("Stack is not initialized."));
  }
  if (level < 0 || static_cast<Word>(level) >= src->getLevelCount()) {
    throw InvalidArgumentException(STR("level"), STR("Core::Data::VariableStack::copyLevel"),
                                   STR("Out of range."));
  }
  this->pushLevel();
  Int count = src->getCount(level);
  if (this->maxStrSize == src->maxStrSize && static_cast<Word>(count) <= this->reservedRecordCount-this->levels.back()) {
    // Copy the whole level at once.
    level -= (src->trunkLevelIndex+1);
    memcpy(this->buffer + this->getRecordSize()*this->levels.back(),
           src->buffer + src->getRecordSize()*(level==0?0:src->levels[level-1]),
           this->getRecordSize()*count);
    this->levels.back() += count;
  } else {
    // Copy variables one by one.
    for (Int i = 0; i < count; ++i) {
      this->add(src->getKey(i, level).c_str(), src->get(i, level));
    }
  }
}


void VariableStack::release()
{
  if (this->buffer) delete[] this->buffer;
  this->buffer = 0;
  this->levels.clear();
}


void VariableStack::setBranchingInfo(VariableStack *vs, Int tli)
{
  if (vs == 0) tli = -1;
  else if (tli < -1 || tli >= static_cast<Int>(vs->getLevelCount())) {
    throw InvalidArgumentException(STR("tli"), STR("Core::Data::VariableStack::setBranchingInfo"),
                                   STR("Must be between -1 and vs->getLevelCount()-1 when vs is not null."));
  }
  this->clear();
  this->trunkStack = vs;
  this->trunkLevelIndex = tli;
}


void VariableStack::ownTopLevel()
{
  ASSERT(this->levels.size() == 0);
  ASSERT(this->trunkStack != 0);
  ASSERT(this->trunkLevelIndex > -1);
  if (static_cast<Int>(this->trunkStack->getLevelCount()) <= this->trunkLevelIndex) {
    throw GeneralException(STR("Trunk stack has been modified."),
                           STR("Core::Data::VariableStack::ownTopLevel"));
  }

  Int srcIndex = this->trunkLevelIndex;
  Word srcStart = srcIndex == 0 ? 0 : this->trunkStack->levels[srcIndex-1];
  Word srcCount = this->trunkStack->levels[srcIndex] - srcStart;

  // If the trunk top level contains more variables than what this stack can hold, resize it.
  if (srcCount > this->reservedRecordCount) {
    Word newReservedRecordCount = this->reservedRecordCount+VariableStack::recordCountIncrement;
    while (srcCount > newReservedRecordCount) {
      newReservedRecordCount += VariableStack::recordCountIncrement;
    }
    this->reinitialize(this->maxStrSize, newReservedRecordCount, this->levels.capacity());
  }

  // Replace trunk level with a new level.
  this->popLevel();
  this->pushLevel();
  this->levels.back() += srcCount;

  // If the trunk top level is empty, we don't need to copy anything.
  if (srcCount == 0) return;

  // If the string size is the same in both stacks, we do a block copy, otherwise we need to copy the variables one by
  // one.
  if (this->trunkStack->maxStrSize == this->maxStrSize) {
    memcpy(this->buffer, this->trunkStack->buffer+srcStart*this->getRecordSize(),
           srcCount*this->getRecordSize());
  } else {
    for (Int i = 0; static_cast<Word>(i) < srcCount; ++i) {
      Byte *destBuf = this->buffer + i*this->getRecordSize();
      Byte *srcBuf = this->trunkStack->buffer + (srcStart+i)*this->trunkStack->getRecordSize();
      sbstr_cast(destBuf).assign(sbstr_cast(srcBuf), this->maxStrSize);
      *reinterpret_cast<IdentifiableObject**>(destBuf+sizeof(Char)*this->maxStrSize) =
          *reinterpret_cast<IdentifiableObject**>(srcBuf+sizeof(Char)*this->trunkStack->maxStrSize);
    }
  }
}


void VariableStack::pushLevel()
{
  if (this->levels.size() == 0) this->levels.push_back(0);
  else this->levels.push_back(this->levels.back());
}


void VariableStack::popLevel()
{
  if (this->levels.size() > 0) {
    this->levels.pop_back();
  } else {
    if (this->trunkLevelIndex >= 0) {
      ASSERT(this->trunkStack != 0);
      this->trunkLevelIndex--;
    } else {
      throw GeneralException(STR("Already empty."), STR("Core::Data::VariableStack::popLevel"));
    }
  }
}


Int VariableStack::add(const Char *key, IdentifiableObject *val)
{
  if (this->levels.size() == 0) {
    if (this->trunkStack == 0 || this->trunkLevelIndex == -1) {
      throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::add"));
    } else {
      this->ownTopLevel();
    }
  }
  if (this->levels.back() >= this->reservedRecordCount) {
    this->reinitialize(this->maxStrSize, this->reservedRecordCount+VariableStack::recordCountIncrement,
                       this->levels.capacity());
  }
  Int start;
  if (this->levels.size() == 1) start = 0;
  else start = this->levels[this->levels.size()-2];
  Byte *buf = this->buffer + this->levels.back()*this->getRecordSize();
  sbstr_cast(buf).assign(key, this->maxStrSize);
  *reinterpret_cast<IdentifiableObject**>(buf+sizeof(Char)*this->maxStrSize) = val;
  this->levels.back()++;
  return this->levels.back() - 1 - start;
}


Int VariableStack::set(const Char *key, IdentifiableObject *val, Bool insertIfNew)
{
  if (this->levels.size() == 0) {
    if (this->trunkStack == 0 || this->trunkLevelIndex == -1) {
      throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::set"));
    } else {
      this->ownTopLevel();
    }
  }
  Int start;
  if (this->levels.size() == 1) start = 0;
  else start = this->levels[this->levels.size()-2];
  Int index = this->findIndex(key, start, this->levels.back()-1);
  if (index == -1) {
    if (insertIfNew) {
      return this->add(key, val);
    } else {
      throw InvalidArgumentException(STR("key"), STR("Core::Data::VariableStack::set"),
                                     STR("Key not found."));
    }
  }
  Byte *buf = this->buffer + (start+index)*this->getRecordSize();
  sbstr_cast(buf).assign(key, this->maxStrSize);
  *reinterpret_cast<IdentifiableObject**>(buf+sizeof(Char)*this->maxStrSize) = val;
  return index;
}


Int VariableStack::getCount(Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::getCount"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::getCount"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->getCount(levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  if (levelIndex == 0) return this->levels[levelIndex];
  else return this->levels[levelIndex] - this->levels[levelIndex-1];
}


IdentifiableObject* VariableStack::get(const Char *key, Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::get"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::get"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->get(key, levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  Int start;
  if (levelIndex == 0) start = 0;
  else start = this->levels[levelIndex-1];
  Int index = this->findIndex(key, start, this->levels[levelIndex]-1);
  if (index == -1) {
    throw InvalidArgumentException(STR("key"), STR("Core::Data::VariableStack::get"),
                                   STR("Key not found."));
  }
  Byte *buf = this->buffer + this->getRecordSize()*(start+index) + sizeof(Char)*this->maxStrSize;
  return *reinterpret_cast<IdentifiableObject**>(buf);
}


IdentifiableObject* VariableStack::get(Int index, Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::get"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::get"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->get(index, levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  Int start;
  if (levelIndex == 0) start = 0;
  else start = this->levels[levelIndex-1];
  if (index < 0 || index >= static_cast<Int>(this->levels[levelIndex])-start) {
    throw InvalidArgumentException(STR("index"), STR("Core::Data::VariableStack::get"),
                                   STR("Out of range."));
  }
  Byte *buf = this->buffer + this->getRecordSize()*(start+index) + sizeof(Char)*this->maxStrSize;
  return *reinterpret_cast<IdentifiableObject**>(buf);
}


const SbStr& VariableStack::getKey(Int index, Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::getKey"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::getKey"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->getKey(index, levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  Int start;
  if (levelIndex == 0) start = 0;
  else start = this->levels[levelIndex-1];
  if (index < 0 || index >= static_cast<Int>(this->levels[levelIndex])-start) {
    throw InvalidArgumentException(STR("index"), STR("Core::Data::VariableStack::getKey"),
                                   STR("Out of range."));
  }
  return sbstr_cast(this->buffer + this->getRecordSize()*(start+index));
}


Int VariableStack::getIndex(const Char *key, Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::getIndex"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::getIndex"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->getIndex(key, levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  Int start;
  if (levelIndex == 0) start = 0;
  else start = this->levels[levelIndex-1];
  Int idx = this->findIndex(key, start, this->levels[levelIndex]-1);
  if (idx == -1) {
    throw InvalidArgumentException(STR("key"), STR("Core::Data::VariableStack::getIndex"),
                                   STR("Not found in the current level."));
  }
  return idx;
}


Int VariableStack::findIndex(const Char *key, Int levelIndex) const
{
  if (this->getLevelCount() == 0) {
    throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::findIndex"));
  }
  if (levelIndex >= static_cast<Int>(this->getLevelCount()) ||
      levelIndex < -static_cast<Int>(this->getLevelCount())) {
    throw InvalidArgumentException(STR("levelIndex"), STR("Core::Data::VariableStack::findIndex"),
                                   STR("Out of range."));
  }
  if (levelIndex < 0) levelIndex += this->getLevelCount();

  if (levelIndex <= this->trunkLevelIndex) return this->trunkStack->findIndex(key, levelIndex);
  else levelIndex -= (this->trunkLevelIndex + 1);

  Int start;
  if (levelIndex == 0) start = 0;
  else start = this->levels[levelIndex-1];
  return this->findIndex(key, start, this->levels[levelIndex]-1);
}


Int VariableStack::findIndex(const Char *key, Int start, Int end) const
{
  ASSERT(this->levels.size() > 0);
  if (static_cast<Word>(start) >= this->levels.back()) return -1;
  for (Int i = start; i <= end; ++i) {
    if (sbstr_cast(this->buffer + this->getRecordSize()*(i)) == key) return i-start;
  }
  return -1;
}


//==============================================================================
// MapPlainContainer Implementation

void VariableStack::set(Int index, IdentifiableObject *val)
{
  if (this->levels.size() == 0) {
    if (this->trunkStack == 0 || this->trunkLevelIndex == -1) {
      throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::set"));
    } else {
      this->ownTopLevel();
    }
  }
  Int start;
  if (this->levels.size() == 1) start = 0;
  else start = this->levels[this->levels.size()-2];
  if (index < 0 || index >= static_cast<Int>(this->levels.back())-start) {
    throw InvalidArgumentException(STR("index"), STR("Core::Data::VariableStack::set"),
                                   STR("Out of range."));
  }
  Byte *buf = this->buffer + (start+index)*this->getRecordSize();
  *reinterpret_cast<IdentifiableObject**>(buf+sizeof(Char)*this->maxStrSize) = val;
}


void VariableStack::remove(Int index)
{
  if (this->levels.size() == 0) {
    if (this->trunkStack == 0 || this->trunkLevelIndex == -1) {
      throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::remove"));
    } else {
      this->ownTopLevel();
    }
  }
  Int start;
  if (this->levels.size() == 1) start = 0;
  else start = this->levels[this->levels.size()-2];
  if (index < 0 || index >= static_cast<Int>(this->levels.back())-start) {
    throw InvalidArgumentException(STR("index"), STR("Core::Data::VariableStack::remove"),
                                   STR("Out of range."));
  }
  Byte *dest = this->buffer + this->getRecordSize()*(start+index);
  Byte *src = dest + this->getRecordSize();
  Int count = this->levels.back() - (start + index);
  memmove(dest, src, this->getRecordSize()*count);
}


void VariableStack::remove(const Char *key)
{
  if (this->levels.size() == 0) {
    if (this->trunkStack == 0 || this->trunkLevelIndex == -1) {
      throw GeneralException(STR("No levels added yet."), STR("Core::Data::VariableStack::remove"));
    } else {
      this->ownTopLevel();
    }
  }
  Int start;
  if (this->levels.size() == 1) start = 0;
  else start = this->levels[this->levels.size()-2];
  Int index = this->findIndex(key, start, this->levels.back()-1);
  if (index == -1) {
    throw InvalidArgumentException(STR("key"), STR("Core::Data::VariableStack::remove"),
                                   STR("Key not found."));
  }
  Byte *dest = this->buffer + this->getRecordSize()*(start+index);
  Byte *src = dest + this->getRecordSize();
  Int count = this->levels.back() - (start + index);
  memmove(dest, src, this->getRecordSize()*count);
}

} } // namespace
