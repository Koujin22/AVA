#pragma once

class AvaCommandResult {

public:
	AvaCommandResult(bool is_off = false, bool cancel = false);
	bool IsOff();
	bool IsCancelled();
	void SetOff(bool off);
	void SetCancelled(bool cancel);
private:
	bool is_off_;
	bool cancel_;

};
