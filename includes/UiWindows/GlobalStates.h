
#ifndef APP_STATES_H
#define APP_STATES_H
#include <unordered_map>



class WinStatesHandler;
class RGStatesHandler;

extern WinStatesHandler win_states;
extern RGStatesHandler win_states;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class WinStates
{
    img_rendr_area,
    rg_grow_panel,
};



int num_windows = 2;

// p
void resolveDependencies(WinStates &states);
void resolveDependencies(WinStates &states);

// Class just to have a cosntructor at hand
class WinStatesHandler
{
private:
    std::unordered_map<WinStates, bool> m_states;
    void resolveDependencies(WinStates &states)
    {
        // Currently don't do anyyhing
    }

public:
    WinStatesHandler()
    {
        m_states.emplace(WinStates(0), false);

        for (int i = 1; i < num_windows; i++)
        {
            m_states.emplace(WinStates(i), false);
        }
    }

    void reverseState(WinStates &state)
    {
        m_states.at(state) = !m_states.at(state);
        resolveDependencies(state);
    }

    bool currentState(WinStates &state)
    {
        return m_states.at(state);
    }
};
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 enum class RGStates
{
    //SEEDStates
     
    //SLICStates
};

int num_rg_states = 0;

// Class just to have a cosntructor at hand
class RGStatesHandler
{
private:
    std::unordered_map<RGStates, bool> m_states;
    void resolveDependencies(RGStates &states)
    {
        // Currently don't do anyyhing
    }

public:
    RGStatesHandler()
    {
        m_states.emplace(RGStates(0), false);

        for (int i = 1; i < num_rg_states; i++)
        {
            m_states.emplace(RGStates(i), false);
        }
    }

    void reverseState(RGStates &state)
    {
        m_states.at(state) = !m_states.at(state);
        resolveDependencies(state);
    }

    bool currentState(RGStates &state)
    {
        return m_states.at(state);
    }
};
 


#endif
