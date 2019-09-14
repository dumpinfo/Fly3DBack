/*! \file */

struct a_star_node
{
	int leaf;
	float cost;
	float estimate;
	a_star_node* parent;
};

//! Transition rule class
/*!
	This class implements a rule that determines a given state transition 
	in a state machine definition. The rule members are: the state over which
	the rule applies, the positive and negative necessary inputs, a factor for 
	randomly evaluating the transition and the output state.
*/
class FLY_ENGINE_API flyStateMachineRule
{
	public:
		int output;			//!< the rule's output state
		int state;			//!< the state over which the rule applies
		int positive_input;	//!< a bitwise indicator of which input logical variables must be true for the transition to occur
		int negative_input;	//!< a bitwise indicator of which input logical variables must be false for the transition to occur
		int factorindex;	//!< the index of the factor (stored in the State Machine) that will be randomly evaluated

	//! Atribuition operator
	void operator=(const flyStateMachineRule& in)
	{
		output=in.output;
		state=in.state;
		positive_input=in.positive_input;
		negative_input=in.negative_input;
		factorindex=in.factorindex;
	}
};

//! Finite state machine class
/*!
	This class implements a finite state machine class, which is made of states,
	inputs and rules for transitions between states. Methods for loading a pre-made
	behaviour file, computing transition and retrieving the current state are provided.
*/
class FLY_ENGINE_API flyStateMachine
{
	protected:
		int nstates;							//!< the total number of states in the machine
		int ninputs;							//!< the number of possible input flags
		int nfactors;							//!< the number of factors for randomization
		flyArray<flyStateMachineRule> rules;	//!< an array of all the transition rules

		int currentstate;

	public:

		//! Default constructor
		flyStateMachine() :
			nstates(0),
			ninputs(0),
			nfactors(0),
			currentstate(-1)
		{ };

		//! Copy-constructor
		flyStateMachine(const flyStateMachine& in);
		
		//! Loads a behaviour from a pre-made file
		int load_behaviour(const char *filename);
		//! Computes transition (if any), with the given inputs and factors
		int transition(int input,float *factors);
		//! Returns the current state
		int get_current_state() { return currentstate; };
};