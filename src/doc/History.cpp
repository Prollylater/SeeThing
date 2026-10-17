#include "History.h"

namespace doc
{
    void History::record(std::unique_ptr<Command> step)
    {
        while (m_steps.size() > m_cursor)
        {
            m_bytes -= m_steps.back()->bytes();
            m_steps.pop_back();
        }
        m_bytes += step->bytes();
        m_steps.push_back(std::move(step));
        m_cursor = m_steps.size();
        trim();
    }

    void History::undo(Document &doc)
    {
        if (canUndo())
            m_steps[--m_cursor]->undo(doc);
    }

    void History::redo(Document &doc)
    {
        if (canRedo())
            m_steps[m_cursor++]->redo(doc);
    }

    void History::clear()
    {
        m_steps.clear();
        m_cursor = 0;
        m_bytes = 0;
        m_dropped = 0;
    }

    void History::setBudget(std::size_t budget)
    {
        m_budget = budget;
        trim();
    }

    // Only applied steps go: an undone step is newer than every applied one, so the oldest is always applied
    void History::trim()
    {
        std::size_t drop = 0;
        while (m_bytes > m_budget && m_cursor - drop > 1)
            m_bytes -= m_steps[drop++]->bytes();
        if (drop == 0)
            return;
        m_steps.erase(m_steps.begin(), m_steps.begin() + std::ptrdiff_t(drop));
        m_cursor -= drop;
        m_dropped += drop;
    }
}
