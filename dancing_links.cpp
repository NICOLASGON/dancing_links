
#include <iostream>
#include <list>
#include <vector>

using namespace std;

class Cell
{
    public:
        Cell *L;
        Cell *R;
        Cell *U;
        Cell *D;
        Cell *C;
        uint32_t S;

        Cell(Cell *horiz, Cell *verti, uint32_t S, Cell *colum)
        {
            this->S = S;
            this->C = colum;
            if( horiz )
            {
                this->L = horiz->L;
                this->R = horiz;
                this->L->R = this;
                this->R->L = this;
            }
            else
            {
                this->L = this;
                this->R = this;
            }
            if( verti )
            {
                this->U = verti->U;
                this->D = verti;
                this->U->D = this;
                this->D->U = this;
            }
            else
            {
                this->U = this;
                this->D = this;
            }
        }

        void hide_verti(void)
        {
            this->U->D = this->D;
            this->D->U = this->U;
        }

        void unhide_verti(void)
        {
            this->D->U = this;
            this->U->D = this;
        }

        void hide_horiz(void)
        {
            this->L->R = this->R;
            this->R->L = this->L;
        }

        void unhide_horiz(void)
        {
            this->R->L = this;
            this->L->R = this;
        }
};


void cover(Cell *c)
{
    c->hide_horiz();
    Cell *pVCell = c->D;
    Cell *pHCell = NULL;
    while( pVCell != c )
    {
        pHCell = pVCell->R;
        while( pHCell != pVCell )
        {
            pHCell->hide_verti();
            pHCell->C->S -= 1;
            pHCell = pHCell->R;
        }
        pVCell = pVCell->D;
    }
}

void uncover(Cell *c)
{
    Cell *pVCell = c->U;
    Cell *pHCell = NULL;
    while( pVCell != c )
    {
        pHCell = pVCell->L;
        while( pHCell != pVCell )
        {
            pHCell->C->S += 1;
            pHCell->unhide_verti();
            pHCell = pHCell->L;
        }
        pVCell = pVCell->U;
    }
    c->unhide_horiz();
}

static bool solve(Cell &header, list<uint32_t> *sol)
{
    if( header.R == &header )
        return true;

    Cell *c = NULL;
    Cell *pHCell = header.R;

    // Recherche de la colonne avec C minimum
    // => C'est la colonne avec le plus de contraintes
    while(pHCell != &header)
    {
        if((c == NULL) || (pHCell->S < c->S))
            c = pHCell;
        pHCell = pHCell->R;
    }

    cover(c);
    Cell *pVCell = c->D;
    while( pVCell != c )
    {
        //////////////////////////////////////
        // On choisit cette solution
        sol->push_back(pVCell->S);
        pHCell = pVCell->R;
        while( pHCell != pVCell )
        {
            cover(pHCell->C);
            pHCell = pHCell->R;
        }
        //////////////////////////////////////
        // Si cette solution est bonne, on sort
        if( solve(header, sol) )
            return true;
        //////////////////////////////////////
        // Sinon on enlève cette solution
        pHCell = pVCell->L;
        while( pHCell != pVCell )
        {
            uncover(pHCell->C);
            pHCell = pHCell->L;
        }
        pVCell = pVCell->D;
        sol->pop_back();
        //////////////////////////////////////
    }
    uncover(c);

    return false;
}

list<uint32_t> * dancing_links(uint32_t size_universe, vector<vector<uint32_t>> &sets)
{
    uint32_t i = 0, j = 0;
    Cell header = Cell(NULL, NULL, 0, NULL);
    vector<Cell*> col;
    for(j=0; j<size_universe; j++)
    {
        Cell *pCell = new Cell(&header, NULL, 0, NULL);
        col.push_back(pCell);
    }
    for(i=0; i<sets.size(); i++)
    {
        Cell *row = NULL;
        for(j=0; j<sets[i].size(); j++)
        {
            col[sets[i][j]]->S += 1;
            row = new Cell(row, col[sets[i][j]], i, col[sets[i][j]]);
        }
    }
    list<uint32_t> *sol = new list<uint32_t>;
    if( solve(header, sol) )
        return sol;
    else
        return NULL;
}

int main(void)
{
    vector<vector<uint32_t>> sets = {
        {2,4,5},
        {0,3,6},
        {1,2,5},
        {0,3},
        {1,6},
        {3,4,6}
    };
    
    list<uint32_t> *sol = dancing_links(7, sets);

    if( sol )
    {
        cout << sol->size() << " sets" << endl;
        while( sol->size() )
        {
            cout << sol->front() << endl;
            sol->pop_front();
        }
    }
    else
    {
        cout << "Pas de solution ..." << endl;
    }

    return 0;
}