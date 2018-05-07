#include <iostream>

struct memory {
    struct memory *next;
    int size;
};

unsigned int memory_size = 256;
struct memory *_memory = nullptr;

void begin_memory() {
    _memory = (memory *) malloc(memory_size);
    _memory->size = memory_size - sizeof(struct memory);
    _memory->next = nullptr;
}

void show_memory() {
    auto *iterator = _memory;
    while (iterator != nullptr) {
        if (iterator->size > 0)
            std::cout << "Avaliable Size:" << iterator->size << std::endl;
        else
            std::cout << (0 - iterator->size) << std::endl;
        iterator = iterator->next;
    }
}

void *first_fit(int size) {
    struct memory *address = nullptr;
    struct memory *previous = nullptr;
    auto *iterator = _memory;
    int _size = size + sizeof(struct memory);

    while (iterator != nullptr) {
        if (iterator->size >= _size) {
            previous = iterator;
            iterator += sizeof(struct memory) + size;
            iterator->size = previous->size - size - (sizeof(struct memory));
            iterator->next = previous->next;
            previous->next = iterator;
            previous->size = 0 - size;
            address = previous + sizeof(struct memory);
        } else {
            if (iterator->size > size) {
                iterator->size = iterator->size * -1;
                address = iterator + sizeof(struct memory);
            }
        }
        iterator = iterator->next;
    }

    return address;
}

struct memory *_next = nullptr;

void *next_fit(int size) {
    struct memory *address = nullptr;
    struct memory *iterator = nullptr;
    struct memory *previous = nullptr;
    int _size = size + sizeof(struct memory);

    if (_next == nullptr)
        iterator = _memory;
    else
        iterator = _next;

    while (iterator != nullptr) {
        if (iterator->size >= _size) {
            previous = iterator;
            iterator += sizeof(struct memory) + size;
            iterator->size = previous->size - size - (sizeof(struct memory));
            iterator->next = previous->next;
            previous->next = iterator;
            previous->size = 0 - size;
            _next = iterator;
            address = previous + sizeof(struct memory);
        } else {
            if (iterator->size > size) {
                iterator->size = iterator->size * -1;
                _next = iterator;
                address = iterator + sizeof(struct memory);
            }
        }
        iterator = iterator->next;
    }

    iterator = _memory;
    while (iterator != _next) {
        if (iterator->size >= _size) {
            previous = iterator;
            iterator += sizeof(struct memory) + size;
            iterator->size = previous->size - size - (sizeof(struct memory));
            iterator->next = previous->next;
            previous->next = iterator;
            previous->size = 0 - size;
            _next = iterator;
            address = previous + sizeof(struct memory);
        } else {
            if (iterator->size > size) {
                iterator->size = iterator->size * -1;
                _next = iterator;
                address = iterator + sizeof(struct memory);
            }
        }
        iterator = iterator->next;
    }

    return address;
}

void *best_fit(int size) {
    struct memory *address = nullptr;
    struct memory *best = nullptr;
    struct memory *next = nullptr;
    auto *iterator = _memory;

    while (iterator != nullptr) {
        if (iterator->size >= size) {
            if (best == nullptr)
                best = iterator;

            if (iterator->size < best->size)
                best = iterator;
        }
        iterator = iterator->next;
    }

    if (best->size == size) {
        best->size = best->size * -1;
        address = best + sizeof(struct memory);
    }

    if (best->size > size) {
        next = best;
        best += sizeof(struct memory) + size;
        best->size = next->size - size;
        best->next = next->next;
        next->next = best;
        next->size = 0 - size;

        address = next + sizeof(struct memory);
    }

    return address;
}

void *worst_fit(int size) {
    struct memory *address = nullptr;
    struct memory *worst = nullptr;
    struct memory *next = nullptr;
    auto *iterator = _memory;

    while (iterator != nullptr) {
        if (iterator->size >= size) {
            if (worst == nullptr)
                worst = iterator;

            if (iterator->size > worst->size)
                worst = iterator;
        }
        iterator = iterator->next;
    }

    std::cout << "Worst: " << worst->size << std::endl;

    if (worst->size == size) {
        worst->size = worst->size * -1;
        address = worst + sizeof(struct memory);
    }

    if (worst->size > size) {
        next = worst;
        worst += sizeof(struct memory) + size;
        worst->size = next->size - size;
        worst->next = next->next;
        next->next = worst;
        next->size = 0 - size;

        address = next + sizeof(struct memory);
    }

    return address;
}

void free(struct memory *address) {
    struct memory *previous = nullptr;
    auto *iterator = _memory;
    address -= sizeof(struct memory);
    address->size = address->size * -1;

    while (iterator != nullptr) {
        if (iterator == address) {
            if (previous != nullptr) {
                if (previous->size > 0) {
                    previous->size += iterator->size;
                    previous->next = iterator->next;
                    if (_next == iterator)
                        _next = previous;
                }
            } else {
                    iterator->size += iterator->next->size;
                    iterator->next = iterator->next->next;
                }
            }
        }

        previous = iterator;
        iterator = iterator->next;
    }
}

int main(int argc, char *argv[]) {
    begin_memory();
    show_memory();
    return 0;
}
