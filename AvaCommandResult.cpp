#include "AvaCommandResult.hpp"

AvaCommandResult::AvaCommandResult(bool is_off, bool cancel) : is_off_{ is_off }, cancel_{ cancel } { }

bool AvaCommandResult::IsCancelled() { return cancel_; }
bool AvaCommandResult::IsOff() { return is_off_; }

void AvaCommandResult::SetCancelled(bool cancel) { cancel_ = cancel; }
void AvaCommandResult::SetOff(bool off) { is_off_ = off; }