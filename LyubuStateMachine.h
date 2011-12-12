#pragma once
#include "ActorStateMachine.h"
class LyubuStateMachine :
	public ActorStateMachine
{
public:
	LyubuStateMachine(void);
	virtual ~LyubuStateMachine(void);
	LyubuStateMachine(ACTORid character);
	//virtual BOOL PlayAction(int skip);
	virtual BOOL PlayAttackAction(int skip);
};

