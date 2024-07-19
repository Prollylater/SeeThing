
#ifndef APP_STATES_H
#define APP_STATES_H
#include <unordered_map>



class WinStatesHandler;
class DisplayStatesHandler;
class RGStatesHandler;

extern WinStatesHandler win_states;
//Horrible name for the 
extern DisplayStatesHandler display_states;
extern RGStatesHandler rg_states;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class WinStates
{
    img_area,
    rg_panel,
};



//int num_windows = 2;


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
        m_states.emplace(WinStates(0), true);

        for (int i = 1; i < 2; i++)
        {
            m_states.emplace(WinStates(i), false);
        }
    }

    void invState(WinStates state)
    {
        m_states.at(state) = !m_states.at(state);
        resolveDependencies(state);
    }

    void setState(WinStates state, bool val)
    {
        m_states.at(state) = val;
        resolveDependencies(state);
    }


    bool isTrue(WinStates state)
    {
        return m_states.at(state);
    }
};
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


enum class MainWinSt
{
    //Tools activation
    displayed_im,
    clr_pencil,
    clr_fill,
};



//int num_mainwin = 2;


// Class just to have a cosntructor at hand
class DisplayStatesHandler
{
private:
    std::unordered_map<MainWinSt, bool> m_states;
    void resolveDependencies(MainWinSt &states)
    {
        // Currently don't do anyyhing
        switch(states){
            case MainWinSt::clr_pencil :
                break;
            case MainWinSt::clr_fill :
            default:
                break;
        }
    }

public:
    DisplayStatesHandler()
    {
        m_states.emplace(MainWinSt(0), false);

        for (int i = 0; i < 2; i++)
        {
            m_states.emplace(MainWinSt(i), false);
        }
    }

    void invState(MainWinSt state)
    {
        m_states.at(state) = !m_states.at(state);
        resolveDependencies(state);
    }

    void setState(MainWinSt state, bool val)
    {
        m_states.at(state) = val;
        resolveDependencies(state);
    }


    bool isTrue(MainWinSt state)
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

//int num_rg_states = 0;

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

        for (int i = 0; i < 0; i++)
        {
            m_states.emplace(RGStates(i), false);
        }
    }

    void invState(RGStates state)
    {
        m_states.at(state) = !m_states.at(state);
        resolveDependencies(state);
    }

    void setState(RGStates state, bool val)
    {
        m_states.at(state) = val;
        resolveDependencies(state);
    }

    bool isTrue(RGStates state)
    {
        return m_states.at(state);
    }

};
 


#endif