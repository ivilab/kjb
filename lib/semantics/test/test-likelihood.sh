#!/bin/bash
COLLINS_DIR="/Users/colindawson/research/src/sac/semspear"

### Arguments
### 1: training trees file within ./trees/
### 2: tagged test sentences within ./tagged/
### 3: low-frequency threshold


# ### Build features for the training sentences
# $COLLINS_DIR/trainCollins ./trees/$1 ./features/train $3

# ### Parse the test sentences using the model trained on training features
# ### The resulting trees are to be used to build features for training
# $COLLINS_DIR/main --collins.model=./features/train --tokenizer.data=$COLLINS_DIR/data/tokenizer ./tagged/$2 1> ./trees/test_from_train.txt

# ### Build features for the test sentences as parsed by the Collins training model
# ### The resulting features are the "training" features for Event_parser
# $COLLINS_DIR/trainCollins ./trees/test_from_train.txt ./features/test_from_train $3

# ### Parse the test sentences using the model trained on test features
# ### The resulting trees and probabilities are reference for the test probabilities
# ### from Event_parser
# $COLLINS_DIR/main --collins.model=./features/test_from_train --tokenizer.data=$COLINS_DIR/data/tokenizer ./tagged/$2 1> ./trees/test_from_test_from_train.txt 2> ./probs/collins_test_from_test_from_train.txt

### Build features for the test sentences parsed by Collins on training
### The resulting features are the "test" features for Event_parser
$COLLINS_DIR/trainCollins ./trees/test_from_test_from_train.txt ./features/test_from_test_from_train $3

### Learn from the training features and evaluate likelihoods for test sentences
./event_parser_test --lexicon.path=./features/test_from_train --train.path=./features/test_from_train --test.path=./features/test_from_test_from_train --lf.thresh=$3 1> ./probs/verify_test_from_test_from_train.txt

#### TINY UNIVERSE TEST

# ### Parse short sentences using the fully trained model.
# $COLLINS_DIR/main --collins.model=./features/train --tokenizer.data=$COLLINS_DIR/data/tokenizer ./tagged/$2 1> ./trees/test_short_out.txt

# ### Build features for short sentences
# $COLLINS_DIR/trainCollins ./trees/test_short_out.txt ./features/test_short $3

# ### Get reference probabilities from Collins parser
# $COLLINS_DIR/main --collins.model=./features/test_short --tokenizer.data=$COLLINS_DIR/data/tokenizer ./tagged/$2 1> ./trees/test_very_short_self.txt 2>./probs/collins_short.txt

# ./event_parser_test --train.path=./features/test_short --test.path=./features/test_short --lf.thresh=$3 > ./probs/verify_test_short.txt