#include "log.hpp"
#include "magic.hpp"

bool magic_match(Storage const& storage) {
  _l("magic_match");

  for (auto i = 0; i < MAGIC_SIZE; ++i) {
    _l(storage.magic[i]);
    _l(MAGIC[i]);
    if (storage.magic[i] != MAGIC[i]) {
      _l("no match!");
      return false;
    }
  }

  _l("matched!");
  return true;
}
