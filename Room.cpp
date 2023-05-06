//
// Created by yegor on 06.05.23.
//

#include "Room.h"

void Room::Join(Room::UserPointer user) {
    users_.insert(user);
    for(const auto& message : message_queue_) {
        user->Deliver(message);
    }
}

void Room::Deliver(const Message &message) {
    message_queue_.push_back(message);
    for(Room::UserPointer const &user : users_) {
        user->Deliver(message);
    }
}

void Room::Leave(Room::UserPointer user) {
    users_.erase(user);
}

