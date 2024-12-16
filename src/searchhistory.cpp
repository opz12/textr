#include "searchhistory.h"


/* Записывает заданный поисковый запрос и его информацию в текущую цепочку истории поиска.
   Если запрос начинает новую цепочку, то история поиска очищается перед вставкой запроса.
   Фактически это означает, что в любой момент времени в истории поиска будет только один элемент.
 */
void SearchHistory::add(QString term, int cursorPositionBeforeFirstSearch, int firstFoundAt)
{
    // Если данный поисковый запрос нарушает текущую "цепочку" поиска, очистить историю

    if (!previouslyFound(term)) {
        searchHistory.clear();
    }

    QPair<int,int> locations;
    locations.first = cursorPositionBeforeFirstSearch;
    locations.second = firstFoundAt;
    searchHistory.insert(term, locations);
}
