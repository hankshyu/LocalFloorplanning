#include <assert.h>
#include "cake.h"
#include "tensor.h"

crust::crust(LFLegaliser *legaliser, Tile *t, double tessCentreX, double tessCentreY): tile(t), crowdIdx(-1), ratingIdx(-1), assignedTessera(nullptr) {
    this->direction = calDirection(tessCentreX, tessCentreY);
    // this->crowdIdx = calRawCrowdIdx(legaliser);
};

double crust::calDirection(double tessCentreX, double tessCentreY){
    Cord tileLL = this->tile->getLowerLeft();
    Cord tileUR = this->tile->getUpperRight();
    double tileCentreX = ((double)(tileLL.x + tileUR.x))/2;
    double tileCentreY = ((double)(tileLL.y + tileUR.y))/2;
    return tns::calVectorAngle(tessCentreX, tessCentreY, tileCentreX, tileCentreY);
};

// double crust::calRawCrowdIdx(LFLegaliser *legaliser){
//     double NeighborsOverlapArea = 0;

//     std::vector <Tile *> neighbors;
//     legaliser->findAllNeighbors(this->tile, neighbors);
    
//     std::vector <Tessera *> surroundingTess;
//     for(Tile *nb : neighbors){
//         if(nb->getType() != tileType::BLANK){
//             std::vector<Tessera *> inTess;
//             legaliser->searchTesseraeIncludeTile(nb, inTess);
//             for(Tessera* t : inTess){
//                 if(!checkVectorInclude(surroundingTess, t)){
//                     surroundingTess.push_back(t);
//                 }
//             }
//         }
//     }

//     for(Tessera *nbTess : surroundingTess){
//         for(Tile *t : nbTess->OverlapArr){
//             NeighborsOverlapArea += (double)(t->getArea());
//         }
//     }

//     return NeighborsOverlapArea;

// };

cake::cake(LFLegaliser *legaliser, std::map <std::string, double> mTessFavorDirection, Tile *overlap, int overlapLV): 
        mLegaliser(legaliser), mOverlap(overlap), mOverlapLevel(overlapLV), mDifficultyIdx(-1) {

    assert(this->mOverlapLevel >= 2);
    assert(this->mOverlapLevel <= 4);
    
    mLegaliser->searchTesseraeIncludeTile(this->mOverlap, this->mMothers);
    assert(this->mMothers.size() == this->mOverlapLevel);
    for(int i = 0; i < mMothers.size(); ++i){
        mMothersFavorDirection.push_back(mTessFavorDirection[mMothers[i]->getName()]);
    }

}

cake::~cake(){
    for(int i = 0; i < 4; ++i){
        for(crust *c : this->surroundings[i]){
            delete(c);
        }
    }
}

double cake::getDifficultyIdx() const{
    return this->mDifficultyIdx;
}

Tile *cake::getOverlapTile() const{
    return this->mOverlap;
}

void cake::collectCrusts(LFLegaliser *legaliser){
    
    // there could be optimisation here: by preknowing the linkings, we may prioritize some blank tiles
    
    for(int i = 0; i < mMothers.size(); ++i){
        Tessera *tess = mMothers[i];
        Cord BBLL = tess->getBBLowerLeft();
        Cord BBUR = tess->getBBUpperRight();

        double tessCentreX = ((double)(BBLL.x + BBUR.x))/2;
        double tessCentreY = ((double)(BBLL.y + BBUR.y))/2;

        std::vector <Cord> record;
        for(Tile *tileInTess : tess->TileArr){
            std::vector <Tile *> Neighbors;
            mLegaliser->findAllNeighbors(tileInTess, Neighbors);
            for(Tile *t : Neighbors){
                bool seenBefore = checkVectorInclude(record, t->getLowerLeft());
                if((t->getType() == tileType::BLANK) && (!seenBefore)){
                    record.push_back(t->getLowerLeft());
                    crust *cr = new crust(legaliser, t, tessCentreX, tessCentreY);
                    this->surroundings[i].push_back(cr);
                }
            }

        }
        for(Tile *tileInTess : tess->OverlapArr){
            std::vector <Tile *> Neighbors;
            mLegaliser->findAllNeighbors(tileInTess, Neighbors);
            for(Tile *t : Neighbors){
                bool seenBefore = checkVectorInclude(record, t->getLowerLeft());
                if((t->getType() == tileType::BLANK) && (!seenBefore)){
                    record.push_back(t->getLowerLeft());
                    crust *cr = new crust(legaliser, t, tessCentreX, tessCentreY);
                    this->surroundings[i].push_back(cr);
                }
            }

        }
    }

    // update the mDifficultyIdx for each cake
    double availBlanks = 0;
    for(int i = 0; i < mMothers.size(); ++i){
        
        Tessera *tess = mMothers[i];
        // fixed blocks could not be distributed
        if(tess->getType() == tesseraType::SOFT){
            for(crust *cst : surroundings[i]){
                availBlanks += cst->tile->getArea();
            }
        }
    }
    this->mDifficultyIdx = availBlanks / this->mOverlap->getArea();


}

void cake::showCake(){
    std::cout << "[C]";
    mOverlap->show(std::cout, false);
    std::cout << ", Difficulty = " << mDifficultyIdx << std::endl;
    std::cout <<"Mothers:" << std::endl;
    for(int i = 0; i < mMothers.size(); ++i){
        Tessera *tess = mMothers[i];
        double fvd = mMothersFavorDirection[i];
        std::cout << tess->getName() << " " << tess->getBBLowerLeft() << " " << tess->getBBUpperRight();
        std::cout << ", FVD = " << fvd << std::endl;

    }
    
    std::cout << std::endl << "Surroundings:" << std::endl;
    for(int i = 0; i <mMothers.size(); ++i){
        std::cout << "Mothers #" << i << std::endl;
        for(crust *c : surroundings[i]){
            std::cout << "(cr)";
            c->tile->show(std::cout, false);
            std::cout << ", Direction: " << c->direction << ", crowdIdx: " << c->crowdIdx << ", rating = " << c->ratingIdx <<std::endl;
        }
    }
}



