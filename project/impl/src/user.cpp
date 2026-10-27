#include "lab_booking/user.hpp"

#include <utility>

namespace lab_booking {

User::User(int id, int departmentId, std::string name)
    : id(id), departmentId(departmentId), name(std::move(name))
{
}

}  // namespace lab_booking
