#ifndef DOC_HISTORY_H
#define DOC_HISTORY_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace doc
{
    class Document;

    // One undoable step. It is recorded after it was applied, so redo() runs only after an undo().
    class Command
    {
    public:
        virtual ~Command() = default;
        virtual std::string label() const = 0;
        virtual void undo(Document &doc) = 0;
        virtual void redo(Document &doc) = 0;
        // What the step keeps alive, for the memory budget
        virtual std::size_t bytes() const { return 0; }
    };

    // The steps in order; the first cursor() of them are applied. A memory budget, not a step count, drops the oldest.
    class History
    {
    public:
        static constexpr std::size_t defaultBudget = std::size_t(512) << 20;

        explicit History(std::size_t budget = defaultBudget) : m_budget(budget) {}

        // Drops the redo steps, then the oldest ones past the budget. The newest step always stays.
        void record(std::unique_ptr<Command> step);
        bool canUndo() const { return m_cursor > 0; }
        bool canRedo() const { return m_cursor < m_steps.size(); }
        void undo(Document &doc);
        void redo(Document &doc);
        void clear();

        std::size_t size() const { return m_steps.size(); }
        std::size_t cursor() const { return m_cursor; }
        const Command &step(std::size_t i) const { return *m_steps.at(i); }
        // Steps dropped by the budget since the last clear
        std::size_t dropped() const { return m_dropped; }
        std::size_t bytes() const { return m_bytes; }
        std::size_t budget() const { return m_budget; }
        void setBudget(std::size_t budget);

    private:
        void trim();

        std::vector<std::unique_ptr<Command>> m_steps;
        std::size_t m_cursor = 0;
        std::size_t m_budget;
        std::size_t m_bytes = 0;
        std::size_t m_dropped = 0;
    };
}

#endif
