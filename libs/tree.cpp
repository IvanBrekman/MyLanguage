//
// Created by IvanBrekman on 09.11.2021.
//

#include "config.h"

#include <cstring>
#include <ctime>

#include "baselib.h"
#include "tree.h"

int tree_ctor(Tree* tree) {
    ASSERT_IF(VALID_PTR(tree), "Invalid tree ptr", 0);

    Node* root = (Node*) calloc_s(1 , sizeof(Node));
    node_ctor(root, NULL, (node_t)INIT_VALUE);

    tree->root  = root;
    tree->size  = 1;
    tree->depth = 0;

    ASSERT_OK(tree, Tree, "Check after ctor call", 0);
    return 1;
}

int tree_dtor(Tree* tree) {
    ASSERT_OK(tree, Tree, "Check before dtor call", 0);

    node_dtor(tree->root);
    tree->size  = -1;
    tree->depth = -1;

    return 1;
}

int node_ctor(Node* node, Node* parent, node_t value) {
    ASSERT_IF(VALID_PTR(node),    "Invalid node ptr",   0);
    ASSERT_IF(VALID_NODE(parent), "Invalid parent ptr", 0);

    node->left_canary  = CANARY;
    node->right_canary = CANARY;

    node->data = value;

    node->parent = parent;
    node->left   = NULL;
    node->right  = NULL;

    node->depth  = 0;

    ASSERT_OK(node, Node, "Check after ctor call", 0);
    return 1;
}

int node_dtor(Node* node) {
    ASSERT_OK(node, Node, "Check before dtor call", 0);
    LOG2(printf("killed node ptr: %p\n", node););

    if (node->left  != NULL) node_dtor(node->left);
    if (node->right != NULL) node_dtor(node->right);

    node->data   = DEINIT_VALUE;
    
    node->parent = (Node*)poisons::FREED_PTR;
    node->left   = (Node*)poisons::FREED_PTR;
    node->right  = (Node*)poisons::FREED_PTR;

    node->depth  = poisons::FREED_ELEMENT;

    node->left_canary  = POISON_CANARY;
    node->right_canary = POISON_CANARY;

    FREE_PTR(node, Node);
    return 1;
}

const char* error_desc(int error_code) {
    switch (error_code)
    {
    case errors::OK_:
        return "ok";
    
    case errors::INVALID_NODE_PTR:
        return "Invalid ptr to Node object";
    case errors::DAMAGED_LCANARY:
        return "Incorrect value of left node canary. May be it isn`t ptr to Node object";
    case errors::DAMAGED_RCANARY:
        return "Incorrect value of right node canary. May be it isn`t ptr to Node object";
    case errors::POISONED_LCANARY:
        return "POISONED value of left node canary. Its ptr to killed Node!!!";
    case errors::POISONED_RCANARY:
        return "POISONED value of left node canary. Its ptr to killed Node!!!";
    case errors::INVALID_PARENT:
        return "Invalid ptr to Node->parent object";
    case errors::INVALID_LEFT:
        return "Invalid ptr to Node->left child object";
    case errors::INVALID_RIGHT:
        return "Invalid ptr to Node->right child object";
    
    case errors::INVALID_TREE_PTR:
        return "Invalid ptr to Tree object";
    case errors::INCORRECT_SIZE:
        return "Incorrect tree size value. Should be (>= 0)";
    case errors::INCORRECT_DEPTH:
        return "Incorrect depth value. Should be (>= 0)";
    
    default:
         return "Unknown value";
    }
}

int Tree_error(Tree* tree) {
    if (!VALID_PTR(tree)) {
        return errors::INVALID_TREE_PTR;
    }

    if (tree->size < 0) {
        return errors::INCORRECT_SIZE;
    }
    if (tree->depth < 0) {
        return errors::INCORRECT_DEPTH;
    }

    int err = Node_error(tree->root, 1);
    if (err) return err;

    return errors::OK_;
}

int Node_error(Node* node, int recursive_check) {
    if (!VALID_PTR(node)) {
        return errors::INVALID_NODE_PTR;
    }

    if (node->left_canary  != CANARY) {
        return node->left_canary  == POISON_CANARY ? errors::POISONED_LCANARY : errors::DAMAGED_LCANARY;
    }
    if (node->right_canary != CANARY) {
        return node->right_canary == POISON_CANARY ? errors::POISONED_RCANARY : errors::DAMAGED_RCANARY;
    }

    if (!VALID_NODE(node->parent)) return errors::INVALID_PARENT;
    if (!VALID_NODE(node->left))   return errors::INVALID_LEFT;
    if (!VALID_NODE(node->right))  return errors::INVALID_RIGHT;

    if (node->depth < 0) {
        return errors::INCORRECT_DEPTH;
    }

    int err = -1;
    if (node->left  != NULL && recursive_check) {
        err = Node_error(node->left,  1);
        if (err) return err;
    }
    if (node->right != NULL && recursive_check) {
        err = Node_error(node->right, 1);
        if (err) return err;
    }

    return errors::OK_;
}

int set_new_root(Tree* tree, Node* new_root) {
    ASSERT_OK(tree,     Tree, "Check before set_new_root func", 0);
    ASSERT_OK(new_root, Node, "Check before set_new_root func", 0);

    node_dtor(tree->root);
    tree->root = new_root;
    update_tree_depth_size(tree);

    ASSERT_OK(tree,     Tree, "Check after set_new_root func",  0);
    return 1;
}

int add_child(Node* parent, Node* child, child_type type) {
    ASSERT_OK(parent, Node, "Check parent before add_child func", 0);

    if (!VALID_PTR(child)) return parent->depth;

    if (type == child_type::LEFT) parent->left  = child;
    else                          parent->right = child;

    child->parent = parent;
    child->depth  = parent->depth + 1;

    return child->depth;
}

int is_leaf(Node* node) {
    ASSERT_OK(node, Node, "Check before is_leaf func", 0);

    return node->left == NULL && node->right == NULL;
}

int is_full_node(Node* node) {
    ASSERT_OK(node, Node, "Check before is_full_node func", 0);

    return node->left != NULL && node->right != NULL;
}

int is_left_child(Node* node) {
    ASSERT_OK(node, Node, "Check before is_left_child func", 0);

    return node->parent != NULL && node->parent->left == node;
}

int is_right_child(Node* node) {
    ASSERT_OK(node, Node, "Check before is_right_child func", 0);

    return node->parent != NULL && node->parent->right == node;
}

Node* find_node_by_value(Tree* tree, node_t value, std::list<NodeDesc>* path, int (*value_comparator) (node_t val1, node_t val2)) {
    ASSERT_OK(tree, Tree, "Invalid tree ptr", NULL);
    ASSERT_IF(VALID_PTR(path), "Invalid path ptr", NULL);

    std::list<Node*> nodes = { };
    get_inorder_nodes(tree->root, &nodes);

    Node* need_node = NULL;
    for (Node* cur_node : nodes) {
        if (value_comparator(cur_node->data, value) == 0) need_node = cur_node;
    }

    if (need_node == NULL) return NULL;

    Node* cur_node = need_node;
    Node* prev_node = NULL;
    while (cur_node != NULL) {
        path->push_front({ cur_node, cur_node->left == prev_node });

        prev_node = cur_node;
        cur_node  = cur_node->parent;
    }

    return need_node;
}

Node* copy_node(Node* node, Node* parent) {
    ASSERT_OK(node, Node, "Check before copy func", NULL);
    ASSERT_IF(VALID_NODE(parent), "Invalip parent ptr", NULL);

    Node* copied_node = (Node*) calloc_s(1, sizeof(Node));
    memcpy(copied_node, node, sizeof(Node));

    copied_node->parent = parent;
    if (VALID_PTR(node->left))   add_child(copied_node, copy_node(node->left,  copied_node), child_type::LEFT);
    if (VALID_PTR(node->right))  add_child(copied_node, copy_node(node->right, copied_node), child_type::RIGHT);

    ASSERT_OK(copied_node, Node, "Check after copy func", NULL);
    return copied_node;
}

int get_inorder_nodes(Node* node, std::list<Node*>* nodes) {
    ASSERT_OK(node, Node, "Check before get_inorder func", 0);
    ASSERT_IF(VALID_PTR(nodes), "Invalid nodes ptr", 0);

    if (node->left  != NULL) get_inorder_nodes(node->left,  nodes);
    nodes->push_back(node);
    if (node->right != NULL) get_inorder_nodes(node->right, nodes);

    return 1;
}

int Node_dump(Node* node, const char* reason, FILE* log) {
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);

    fprintf(log, "Node <");
    fprintf(log, COLORED_OUTPUT("%p", PURPLE, log), node);
    fprintf(log, ">----------------------------------------------------------\n");
    FPRINT_DATE(log);
    fprintf(log, "\n");

    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);
    int err = Node_error(node);

    fprintf(log, " Node state:    %d ", err);
    if (err != 0) fprintf(log, COLORED_OUTPUT("(%s)\n", RED,   log), error_desc(err));
    else          fprintf(log, COLORED_OUTPUT("(%s)\n", GREEN, log), error_desc(err));

    if (!VALID_PTR(node)) return 0;

    fprintf(log, " Depth in tree: %d %s\n\n", node->depth, node->depth < 0 ? COLORED_OUTPUT("(BAD)", RED, log) : "");

    fprintf(log, "  Left canary: %lX %s\n"
                 " Right canary: %lX %s\n\n",
            node->left_canary,  node->left_canary  == CANARY ? "" : node->left_canary  == POISON_CANARY ? COLORED_OUTPUT("(POISON)", PURPLE, log) : COLORED_OUTPUT("(BAD)", RED, log),
            node->right_canary, node->right_canary == CANARY ? "" : node->right_canary == POISON_CANARY ? COLORED_OUTPUT("(POISON)", PURPLE, log) : COLORED_OUTPUT("(BAD)", RED, log)
    );

    fprintf(log, " Parent: %p %s\n"
                 " Left:   %p %s\n"
                 " Right:  %p %s\n\n",
            node->parent, VALID_PTR(node->parent) ? "" : node->parent == NULL ? COLORED_OUTPUT("NULL", BLUE, log) : COLORED_OUTPUT("(BAD)", RED, log),
            node->left,   VALID_PTR(node->left)   ? "" : node->left   == NULL ? COLORED_OUTPUT("NULL", BLUE, log) : COLORED_OUTPUT("(BAD)", RED, log),
            node->right,  VALID_PTR(node->right)  ? "" : node->right  == NULL ? COLORED_OUTPUT("NULL", BLUE, log) : COLORED_OUTPUT("(BAD)", RED, log)
    );

    fprintf(log, " Data: type - %2d\n", node->data.type);
    switch (node->data.type) {
        case data_type::CONST_T:
            fprintf(log, "      value - %d\n", node->data.value.number);
            break;
        case data_type::VAR_T:
        case data_type::OPR_T:
            fprintf(log, "      value - '%s'\n", node->data.value.name);
            break;
        
        default:
            fprintf(log, COLORED_OUTPUT("      Unknown data type\n", RED, log));
    }

    fprintf(log, "-------------------------------------------------------------------------------\n");

    return 1;
}

int Tree_dump(Tree* tree, const char* reason, FILE* log) {
    ASSERT_IF(VALID_PTR(tree),   "Invalid tree ptr",   0);
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);

    fprintf(log, COLORED_OUTPUT("|-------------------------          Tree  Dump          -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);
    int err = Tree_error(tree);

    fprintf(log, "    Tree state: %d ", err);
    if (err != 0) fprintf(log, COLORED_OUTPUT("(%s)\n\n", RED,   log), error_desc(err));
    else          fprintf(log, COLORED_OUTPUT("(%s)\n\n", GREEN, log), error_desc(err));

    fprintf(log, "Tree size:  %d %s\n"
                 "Tree depth: %d %s\n\n",
            tree->size,  tree->size  < 0 ? COLORED_OUTPUT("(BAD)", RED, log) : "",
            tree->depth, tree->depth < 0 ? COLORED_OUTPUT("(BAD)", RED, log) : ""
    );

    std::list<Node*> nodes = { };

    nodes.push_back(tree->root);
    while (!nodes.empty()) {
        Node* pop_node = nodes.front();
        nodes.pop_front();

        if (!VALID_PTR(pop_node)) {
            fprintf(log, COLORED_OUTPUT("Cant access node <%p>\n", RED, log), pop_node);
            continue;
        }

        if (pop_node->left  != NULL) nodes.push_back(pop_node->left);
        if (pop_node->right != NULL) nodes.push_back(pop_node->right);

        int exit_code = Node_dump(pop_node, reason, log);
        if (exit_code != 1) fprintf(log, COLORED_OUTPUT("Node_dump bad exit code (%d)", RED, log), exit_code);
    }

    nodes.~list();

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}

int write_tree_to_file(Tree* tree, const char* filename, int w_type) {
    ASSERT_OK(tree, Tree, "Check before write_tree_to_file func", 0);
    ASSERT_IF(VALID_PTR(filename), "Invalid filename ptr", 0);

    FILE* file = open_file(filename, "w");

    switch (w_type) {
        case write_type::PREORDER:
            preorder_write_nodes_to_file(tree->root, file);
            break;
        case write_type::INORDER:
            inorder_write_nodes_to_file(tree->root, file);
            break;

        default:
            PRINT_WARNING("Unknown write type. Nothing is writting\n");
            break;
    }
    fputs("\n", file);
    
    fclose(file);
    return 1;
}

int preorder_write_nodes_to_file(Node* node, FILE* file) {
    ASSERT_OK(node, Node, "Invalid node ptr", 0);
    ASSERT_IF(VALID_PTR(file), "Invalid file ptr", 0);

    SPR_FPUTS(file, "{ \"%c\" ", node->data.value.number);

    if (node->left  != NULL) preorder_write_nodes_to_file(node->left,  file);
    else fputs("n ", file);

    if (node->right != NULL) preorder_write_nodes_to_file(node->right, file);
    else fputs("n ", file);

    fputs("} ", file);

    return 1;
}

int inorder_write_nodes_to_file(Node* node, FILE* file) {
    ASSERT_OK(node, Node, "Invalid node ptr", 0);
    ASSERT_IF(VALID_PTR(file), "Invalid file ptr", 0);

    if (node->depth != 0) fputs("(", file);

    if (node->left  != NULL) inorder_write_nodes_to_file(node->left,  file);
    SPR_FPUTS(file, " %c ", node->data.value.number);
    if (node->right != NULL) inorder_write_nodes_to_file(node->right, file);

    if (node->depth != 0) fputs(")", file);

    return 1;
}

int update_tree_depth_size(Tree* tree) {
    ASSERT_OK(tree, Tree, "Chek before update_tree_depth_size func", 0);

    int size = 0;
    std::list<Node*> nodes = { tree->root };
    while (!nodes.empty()) {
        Node* cur_node = nodes.front();
        
        cur_node->depth = cur_node->parent != NULL ? cur_node->parent->depth + 1 : 0;
        tree->depth = cur_node->depth;
        nodes.pop_front();

        if (cur_node->left  != NULL) nodes.push_back(cur_node->left);
        if (cur_node->right != NULL) nodes.push_back(cur_node->right);

        size++;
    }
    tree->size = size;

    ASSERT_OK(tree, Tree, "Chek after update_tree_depth_size func", 0);
    return 1;
}

int Tree_dump_graph(Tree* tree, const char* reason, FILE* log, int show_parent_edge) {
    ASSERT_IF(VALID_PTR(tree),   "Invalid tree ptr",   0);
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);

    FILE* dot_file = open_file("logs/dot_file.txt", "w");

    fputs("digraph structs {\n", dot_file);
    fputs("\trankdir=HR\n"
          "\tlabel=\"", dot_file);
    fputs(reason, dot_file);
    fputs("\"\n\n", dot_file);

    int size  = tree->size;
    int depth = tree->depth;
    Node* root = tree->root;

    // Fill data to graphiz----------------------------------------------------
    SPR_FPUTS(dot_file, "\tdepth[ shape=component label=\"depth: %d\" ]\n"
                        "\t size[ shape=component label=\"size:  %d\" ]\n",
              depth, size
    );

    fputs("\n\t{\n\t\tnode[ style=invis ]\n\t\tedge[ style=invis ]\n", dot_file);
    for (int i = 0; i <= depth; i++) {
        SPR_FPUTS(dot_file, "\t\t%d -> %d\n", i - 1, i);
    }
    fputs("\t}\n\n", dot_file);

    std::list<Node*> nodes = { };
    get_inorder_nodes(root, &nodes);

    for (Node* cur_node : nodes) {
        int        type  = cur_node->data.type;
        data_value value = cur_node->data.value;

        const char* color = "black";
        if (type == data_type::ERROR_T) color = "red";
        if (type == data_type::CONST_T) color = "green";
        if (type == data_type::VAR_T)   color = "maroon";
        if (type == data_type::OPR_T)   color = "magenta2";

        const char* shape = cur_node->data.saving_node == 0 ? "record" : "ellipse";

        if (type == data_type::OPR_T && (cur_node->data.value.name[0] == '<' || cur_node->data.value.name[0] == '>'))
                                             SPR_FPUTS(dot_file, "\t%d[ shape=%s label=\"\\%s\" width=2 fontsize=25 style=\"filled\" color=\"%s\" ]\n", INT_ADDRESS(cur_node), shape, cur_node->data.value.name,   color);
        else if (type == data_type::CONST_T) SPR_FPUTS(dot_file, "\t%d[ shape=%s label=\"%d\" width=2 fontsize=25 style=\"filled\" color=\"%s\" ]\n", INT_ADDRESS(cur_node), shape, cur_node->data.value.number, color);
        else if (type == data_type::OPR_T ||
                 type == data_type::VAR_T)   SPR_FPUTS(dot_file, "\t%d[ shape=%s label=\"%s\" width=2 fontsize=25 style=\"filled\" color=\"%s\" ]\n", INT_ADDRESS(cur_node), shape, cur_node->data.value.name,   color);
        else                                 SPR_FPUTS(dot_file, "\t%d[ shape=%s label=\"UN\" width=2 fontsize=25 style=\"filled\" color=\"%s\" ]\n", INT_ADDRESS(cur_node), shape, color);

        if (cur_node->parent != NULL && show_parent_edge) {
            SPR_FPUTS(dot_file, "\t%d -> %d\n", INT_ADDRESS(cur_node), INT_ADDRESS(cur_node->parent));
        }
        if (cur_node->left   != NULL) {
            SPR_FPUTS(dot_file, "\t%d -> %d[ label=\"L\" ]\n", INT_ADDRESS(cur_node), INT_ADDRESS(cur_node->left));
        }
        if (cur_node->right  != NULL) {
            SPR_FPUTS(dot_file, "\t%d -> %d[ label=\"R\" ]\n", INT_ADDRESS(cur_node), INT_ADDRESS(cur_node->right));
        }
        fputs("\n", dot_file);
    }

    nodes.clear();
    nodes.push_back(root);

    int cur_depth = -1;
    while (!nodes.empty()) {
        Node* pop_node = nodes.front();
        nodes.pop_front();

        if (pop_node->left  != NULL) nodes.push_back(pop_node->left);
        if (pop_node->right != NULL) nodes.push_back(pop_node->right);

        if (pop_node->depth != cur_depth) {
            cur_depth = pop_node->depth;
            SPR_FPUTS(dot_file, "%s\t{ rank = same; %d; %d;", pop_node == root ? "" : " }\n", cur_depth, INT_ADDRESS(pop_node));
        } else {
            SPR_FPUTS(dot_file, " %d;", INT_ADDRESS(pop_node));
        }
    }
    fputs(" }\n", dot_file);
    

    nodes.~list();
    // ------------------------------------------------------------------------

    fputs("}\n", dot_file);
    fclose(dot_file);

    time_t seconds = time(NULL);
    SPR_SYSTEM("dot -Tpng logs/dot_file.txt -o logs/graph_%ld.png", seconds);

    fputs("<h1 align=\"center\">Dump Tree</h1>\n<pre>\n", log);
    Tree_dump(tree, reason, log);
    SPR_FPUTS(log, "</pre>\n<img src=\"graph_%ld.png\">\n\n", seconds);

    return (int) seconds;
}
