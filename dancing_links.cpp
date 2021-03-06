
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>

using namespace std;

const char header_file[] = "digraph g {\n"
                           "graph [ rankdir = \"TB\" ];\n"
                           "node  [ fontsize = \"16\" shape = \"record\" ];\n"
                           "edge [ ];\n";
const char footer_file[] = "}\n";

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

// Couvrir une colonne consiste à temporairement enlever cette colonne de 
// la liste chaînée des colonnes, et pour chaque case de la colonne 
// retirer les autres cellules qui sont sur la même ligne.
//
// Cette manoeuvre signifie que l'on a choisit un set dans cette colonne.
// Les autres sets qui inclus cette colonnes ne peuvent pas être choisis
// ainsi que tous les élements de ces sets donc on retire tout temporairement.
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

// Cette opération est le dual de la fonction cover et permet de tout remettre
// en place de façon à annuler la suppresion. C'est ce mécanisme qui permet de
// faire du backtracking.
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

// La recherche de la solution est une méthode récursive via
// backtracking. Cette fonction est symetrique, elle sélectionne
// la colonne la plus contrainte <=> avec le moins d'élements dans
// le colonne. Elle va parcourir cette colonne de haut en bas en
// sélectionnant l'entrée et en couvrant la colonne ce qui revient
// à créer une nouvelle instance du problème.
// Ce parcours récursif s'arrète quand il reste une colonne qui
// n'a pas été choisie mais qui est vide.
// Celà signifie que les choix que l'on a fait précédement nous 
// ont ammené dans une impasse car une solution à ce problème se
// doit d'avoir un ensemble de sets qui couvre toutes les colonnes.
// Au moment où l'on détecte une impasse comme la colonne est vide 
// l'algorithme ne rentre pas dans le while et sort directement en
// retournant false ce qui signifie aux instances du dessus que 
// cette solution est une impasse et ils font marche arrière en 
// testant les autres entrées de la colonne.
static bool solve(Cell &header, vector<uint32_t> *sol)
{
    // Si toutes les colonnes on été choisies c'est
    // que nous avons trouvé une solution.
    // On retourne true afin de terminer toutes les 
    // instances du dessus.
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

    //////////////////////////////////////
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
    //////////////////////////////////////

    return false;
}

void print_header(Cell &header, string filename)
{
    ofstream file(filename, ofstream::out);

    if(file)
    {
        file << header_file;

        Cell *pHCell = &header;
        Cell *pVCell = pHCell->D;
    
        file << "{ rank = same; ";
        do
        {
            file << " \"" << pHCell << "\";";
            pHCell = pHCell->R;
        }
        while( pHCell != &header );
        file << " }" << endl;

        pHCell = header.R;
        pVCell = pHCell->D;

        file << "\"" << &header << "\" [ label = \"<f0> "  << &header <<"\" shape = \"record\" ];" << endl;
        file << "\"" << &header << "\" -> \"" << header.U << "\" [color = green]" << endl;
        file << "\"" << &header << "\" -> \"" << header.D << "\" [color = cyan]" << endl;
        file << "\"" << &header << "\" -> \"" << header.L << "\" [color = blue]" << endl;
        file << "\"" << &header << "\" -> \"" << header.R << "\" [color = red]" << endl;
    
        while( pHCell != &header )
        {
            file << "\"" << pHCell << "\" [ label = \"<f0> "  << pHCell <<"\" shape = \"record\" ];" << endl;
            file << "\"" << pHCell << "\" -> \"" << pHCell->U << "\" [color = green]" << endl;
            file << "\"" << pHCell << "\" -> \"" << pHCell->D << "\" [color = cyan]" << endl;
            file << "\"" << pHCell << "\" -> \"" << pHCell->L << "\" [color = blue]" << endl;
            file << "\"" << pHCell << "\" -> \"" << pHCell->R << "\" [color = red]" << endl;

            pVCell = pHCell->D;
            while( pVCell != pHCell )
            {
                file << "\"" << pVCell << "\" [ label = \"<f0> "  << pVCell <<"\" shape = \"record\" ];" << endl;
                file << "\"" << pVCell << "\" -> \"" << pVCell->U << "\" [color = green]" << endl;
                file << "\"" << pVCell << "\" -> \"" << pVCell->D << "\" [color = cyan]" << endl;
                file << "\"" << pVCell << "\" -> \"" << pVCell->L << "\" [color = blue]" << endl;
                file << "\"" << pVCell << "\" -> \"" << pVCell->R << "\" [color = red]" << endl;
                pVCell = pVCell->D;
            }
            pHCell = pHCell->R;
        }

        file << footer_file;

        file.close();
    }
    else
    {
        cout << "Erreur fichier" << endl;
    }
}

// Fonction qui met en forme le problème et le résouds.
// Cette fonction retourne une liste de set ou NULL si il n'y a pas de solution.
vector<uint32_t> * dancing_links(uint32_t size_universe, vector<vector<uint32_t>> &sets)
{
    uint32_t i = 0, j = 0;
    bool IsASolution = false;
    list<Cell*> lCell;
    Cell *pCell = NULL;

    // Allocation des colonnes
    vector<Cell*> col;
    Cell header = Cell(NULL, NULL, 0, NULL);
    for(j=0; j<size_universe; j++)
    {
        pCell = new Cell(&header, NULL, 0, NULL);
        col.push_back(pCell);
        lCell.push_back(pCell);
    }

    // Allocation des cellules
    for(i=0; i<sets.size(); i++)
    {
        Cell *row = NULL;
        for(j=0; j<sets[i].size(); j++)
        {
            col[sets[i][j]]->S += 1;
            row = new Cell(row, col[sets[i][j]], i, col[sets[i][j]]);
            lCell.push_back(row);
        }
    }
    
    vector<uint32_t> *sol = new vector<uint32_t>;

    print_header(header, "01.dot");

    // Résolution du problème
    IsASolution = solve(header, sol);

    // Libère toute les cellules allouées
    for(list<Cell*>::iterator it = lCell.begin(); it != lCell.end(); ++it)
        delete *it;

    if( IsASolution )
        return sol;
    else
        return NULL;
}

// Programme d'exemple
int main(void)
{
    uint32_t i = 0;
    vector<vector<uint32_t>> sets = {
        {2,4,5},
        {0,3,6},
        {1,2,5},
        {0,3},
        {1,6},
        {3,4,6}
    };
    
    vector<uint32_t> *sol = dancing_links(7, sets);

    if( sol )
    {
        cout << sol->size() << " sets" << endl;
        for(i=0; i < sol->size(); i++ )
        {
            cout << (*sol)[i] << endl;
        }
        delete sol;
    }
    else
    {
        cout << "Pas de solution ..." << endl;
    }

    return 0;
}
