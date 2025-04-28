#include <stdio.h> // Giri�/��k�� i�lemleri i�in standart k�t�phane
#include <stdlib.h> // Bellek y�netimi ve genel ama�l� fonksiyonlar i�in standart k�t�phane
#include <string.h> // String i�lemleri i�in standart k�t�phane

#define MAX_USERS 1000 // Maksimum kullan�c� say�s�

// Kullan�c� Yap�s�
typedef struct User {
    int id; // Kullan�c�n�n ID'si
    int friendCount; // Arkada� say�s�
    struct User** friends; // Kullan�c�n�n arkada�lar�n�n listesi
} User;

// Red-Black Tree Node Yap�s�
typedef enum { RED, BLACK } Color; // D���m renklerini temsil eden enum

typedef struct RBNode {
    int id; // D���m�n temsil etti�i kullan�c�n�n ID'si
    User* user; // D���m�n i�aret etti�i kullan�c�
    Color color; // D���m�n rengi (k�rm�z� veya siyah)
    struct RBNode* left; // Sol �ocuk d���m
    struct RBNode* right; // Sa� �ocuk d���m
    struct RBNode* parent; // �st (ebeveyn) d���m
} RBNode;

// Global De�i�kenler
User* users[MAX_USERS]; // Kullan�c�lar� tutan dizi
int userCount = 0; // Kullan�c� sayac�
RBNode* root = NULL; // Red-Black Tree'nin k�k�

// Fonksiyon �mza Bildirimleri
User* createUser(int id); // Yeni kullan�c� olu�turur
void addFriendship(int id1, int id2); // �ki kullan�c� aras�nda arkada�l�k olu�turur
User* getUser(int id); // ID ile kullan�c� arar
void readDataset(const char* filename); // Veriseti dosyas�n� okur
void dfs(User* user, int currentDepth, int targetDepth, int* visited, int* found, int* foundCount); // DFS algoritmas�
void findFriendsAtDistance(int id, int distance); // Belirli mesafedeki arkada�lar� bulur
void findCommonFriends(int id1, int id2); // �ki kullan�c� aras�ndaki ortak arkada�lar� bulur
void exploreCommunity(User* user, int* visited, int* community, int* communitySize); // Toplulu�u ke�feder
void detectCommunities(); // Topluluklar� tespit eder
void dfsInfluence(User* user, int* visited, int* count); // Etki alan� i�in DFS algoritmas�
void calculateInfluence(int id); // Kullan�c�n�n etki alan�n� hesaplar
void rbInsert(User* user); // Red-Black Tree'ye kullan�c� ekler
void rbInsertFixup(RBNode* node); // Red-Black Tree'yi d�zeltir
void leftRotate(RBNode* x); // Red-Black Tree'de sola d�nd�rme
void rightRotate(RBNode* y); // Red-Black Tree'de sa�a d�nd�rme
RBNode* createRBNode(User* user); // Red-Black Tree d���m� olu�turur
User* rbSearch(int id); // Red-Black Tree i�inde kullan�c� arar

// Kullan�c� olu�tur
User* createUser(int id) {
    User* user = (User*)malloc(sizeof(User)); // Kullan�c� i�in haf�za ay�r
    user->id = id; // ID'yi ata
    user->friendCount = 0; // Ba�lang��ta arkada� say�s� 0
    user->friends = (User**)malloc(sizeof(User*) * MAX_USERS); // Arkada�lar dizisi i�in haf�za ay�r
    return user; // Kullan�c�y� d�nd�r
}

// ID ile kullan�c�y� getir
User* getUser(int id) {
    int i = 0; // Saya� de�i�keni
    for (i = 0; i < userCount; i++) { // T�m kullan�c�lar i�inde dola�
        if (users[i]->id == id) { // ID e�le�irse
            return users[i]; // Kullan�c�y� d�nd�r
        }
    }
    return NULL; // Bulunamazsa NULL d�nd�r
}

// �ki kullan�c� aras�nda arkada�l�k kur
void addFriendship(int id1, int id2) {
    User* user1 = getUser(id1); // �lk kullan�c�y� bul
    User* user2 = getUser(id2); // �kinci kullan�c�y� bul
    if (user1 && user2) { // �kisi de varsa
        user1->friends[user1->friendCount++] = user2; // user2'yi user1'in arkada� listesine ekle
        user2->friends[user2->friendCount++] = user1; // user1'i user2'nin arkada� listesine ekle
    }
}

// Veriseti dosyas�n� oku
void readDataset(const char* filename) {
    FILE* file = fopen(filename, "r"); // Dosyay� oku modunda a�
    if (!file) { // Dosya a��lamad�ysa
        printf("Dosya a�ilamadi.\n"); // Hata mesaj� ver
        return; // Fonksiyondan ��k
    }
    
    char line[100]; // Sat�r okuma buffer'�
    while (fgets(line, sizeof(line), file)) { // Sat�r sat�r oku
        char type[10]; // Sat�r�n tipini sakla (USER ya da FRIEND)
        int id1, id2; // Kullan�c� ID'leri
        sscanf(line, "%s", type); // Sat�r tipini oku
        if (strcmp(type, "USER") == 0) { // Sat�r tipi USER ise
            sscanf(line, "%*s %d", &id1); // ID'yi al
            User* user = createUser(id1); // Yeni kullan�c� olu�tur
            users[userCount++] = user; // Kullan�c�y� listeye ekle
            rbInsert(user); // Red-Black Tree'ye kullan�c�y� ekle
        } else if (strcmp(type, "FRIEND") == 0) { // Sat�r tipi FRIEND ise
            sscanf(line, "%*s %d %d", &id1, &id2); // �ki ID'yi oku
            addFriendship(id1, id2); // �ki kullan�c� aras�nda arkada�l�k olu�tur
        }
    }
    fclose(file); // Dosyay� kapat
}

// DFS algoritmas� (Depth First Search)
void dfs(User* user, int currentDepth, int targetDepth, int* visited, int* found, int* foundCount) {
    int i = 0; // Saya� de�i�keni
    if (!user || visited[user->id]) return; // Kullan�c� yoksa veya ziyaret edildiyse ��k
    visited[user->id] = 1; // Kullan�c�y� ziyaret edildi olarak i�aretle
    
    if (currentDepth == targetDepth) { // Hedef derinli�e ula��ld�ysa
        found[(*foundCount)++] = user->id; // Bulunan kullan�c� listesine ekle
        return; // Daha ileri gitme
    }
    
    for (i = 0; i < user->friendCount; i++) { // Arkada�lar aras�nda dola�
        dfs(user->friends[i], currentDepth + 1, targetDepth, visited, found, foundCount); // Derinli�i art�rarak DFS uygula
    }
}

// Belirli mesafedeki arkada�lar� bulur
void findFriendsAtDistance(int id, int distance) {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int found[MAX_USERS]; // Bulunan kullan�c�lar
    int foundCount = 0; // Bulunan kullan�c� say�s�
    int i = 0; // Saya� de�i�keni
    
    User* user = getUser(id); // Kullan�c�y� bul
    if (!user) { // Kullan�c� bulunamad�ysa
        printf("Kullanici bulunamadi.\n"); // Hata mesaj� ver
        return; // Fonksiyondan ��k
    }
    
    dfs(user, 0, distance, visited, found, &foundCount); // DFS ile kullan�c�lar� ara
    
    printf("%d mesafedeki arkadaslar: ", distance); // Sonu�lar� yazd�r
    for (i = 0; i < foundCount; i++) { // Bulunanlar� yazd�r
        printf("%d ", found[i]);
    }
    printf("\n");
}

// Ortak arkada�lar� bulur
void findCommonFriends(int id1, int id2) {
    User* user1 = getUser(id1); // �lk kullan�c�y� bul
    User* user2 = getUser(id2); // �kinci kullan�c�y� bul
    int i = 0, j = 0; // Saya� de�i�kenleri
    
    if (!user1 || !user2) { // Kullan�c�(lar) bulunamad�ysa
        printf("Kullanici(lar) bulunamadi.\n"); // Hata mesaj� ver
        return; // Fonksiyondan ��k
    }
    
    printf("%d ve %d ortak arkadaslari: ", id1, id2); // Ba�l�k yaz
    for (i = 0; i < user1->friendCount; i++) { // user1'in arkada�lar�nda dola�
        for (j = 0; j < user2->friendCount; j++) { // user2'nin arkada�lar�nda dola�
            if (user1->friends[i]->id == user2->friends[j]->id) { // Ortak arkada� bulunduysa
                printf("%d ", user1->friends[i]->id); // Ortak arkada�� yazd�r
            }
        }
    }
    printf("\n");
}

void exploreCommunity(User* user, int* visited, int* community, int* communitySize) {
    int i = 0; // Saya� de�i�keni
    if (visited[user->id]) return; // Kullan�c� daha �nce ziyaret edildiyse ��k
    visited[user->id] = 1; // Kullan�c�y� ziyaret edildi olarak i�aretle
    community[(*communitySize)++] = user->id; // Kullan�c�y� toplulu�a ekle
    
    for (i = 0; i < user->friendCount; i++) { // Kullan�c�n�n arkada�lar�nda dola�
        exploreCommunity(user->friends[i], visited, community, communitySize); // Arkada�lar�n� da ziyaret et
    }
}

// Topluluklar� tespit eder
void detectCommunities() {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int i = 0; // Saya� de�i�keni
    int community[MAX_USERS]; // Toplulu�u tutacak dizi
    int communitySize = 0; // Toplulu�un b�y�kl���
    
    printf("Topluluklar:\n"); // Ba�l�k yazd�r
    
    for (i = 0; i < userCount; i++) { // T�m kullan�c�lar �zerinde d�ng� ba�lat
        if (!visited[users[i]->id]) { // Kullan�c� hen�z ziyaret edilmediyse
            communitySize = 0; // Yeni topluluk olu�turulacak
            exploreCommunity(users[i], visited, community, &communitySize); // Toplulu�u ke�fet
            
            int j = 0; // Saya� de�i�keni
            printf("[ "); // Topluluklar� yazd�rmaya ba�la
            for (j = 0; j < communitySize; j++) { // Toplulu�u yazd�r
                printf("%d ", community[j]);
            }
            printf("]\n"); // Toplulu�un biti�i
        }
    }
}

// Etki alan� i�in DFS algoritmas�
void dfsInfluence(User* user, int* visited, int* count) {
    int i = 0; // Saya� de�i�keni
    if (visited[user->id]) return; // E�er kullan�c�y� zaten ziyaret ettiysek ��k
    visited[user->id] = 1; // Kullan�c�y� ziyaret edildi olarak i�aretle
    (*count)++; // Etki alan� say�s�n� art�r
    
    for (i = 0; i < user->friendCount; i++) { // Kullan�c�n�n arkada�lar�nda dola�
        dfsInfluence(user->friends[i], visited, count); // Derinlemesine gez
    }
}

// Kullan�c�n�n etki alan�n� hesaplar
void calculateInfluence(int id) {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int count = 0; // Etki alan� sayac�
    
    User* user = getUser(id); // Kullan�c�y� bul
    if (!user) { // Kullan�c� bulunamazsa
        printf("Kullanici bulunamadi.\n"); // Hata mesaj� ver
        return; // Fonksiyondan ��k
    }
    
    dfsInfluence(user, visited, &count); // Etki alan� hesaplamak i�in DFS uygula
    printf("%d numarali kullanicinin etki alani: %d\n", id, count - 1); // Sonucu yazd�r (kendisi dahil edilmemeli)
}

// Red-Black Tree i�lemleri

// Yeni Red-Black Tree d���m� olu�turur
RBNode* createRBNode(User* user) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode)); // D���m i�in haf�za ay�r
    node->id = user->id; // Kullan�c� ID'sini d���me ata
    node->user = user; // Kullan�c�y� d���me ata
    node->color = RED; // D���m rengini k�rm�z� olarak ata
    node->left = node->right = node->parent = NULL; // D���m�n �ocuklar�n� ve ebeveynini NULL yap
    return node; // Yeni d���m� d�nd�r
}

// Red-Black Tree'de sola d�nd�rme
void leftRotate(RBNode* x) {
    RBNode* y = x->right; // Y d���m�n� x'in sa� �ocu�u olarak ata
    x->right = y->left; // Y'nin sol �ocu�unu x'in sa� �ocu�u yap
    if (y->left) y->left->parent = x; // E�er Y'nin sol �ocu�u varsa, ebeveynini x olarak ata
    y->parent = x->parent; // Y'nin ebeveynini x'in ebeveyni olarak ata
    if (!x->parent) root = y; // E�er x'in ebeveyni yoksa, root'u y olarak ata
    else if (x == x->parent->left) x->parent->left = y; // x sol �ocuksa, ebeveynin sol �ocu�u y olsun
    else x->parent->right = y; // x sa� �ocuksa, ebeveynin sa� �ocu�u y olsun
    y->left = x; // x'i y'nin sol �ocu�u yap
    x->parent = y; // x'in ebeveynini y olarak ata
}

// Red-Black Tree'de sa�a d�nd�rme
void rightRotate(RBNode* y) {
    RBNode* x = y->left; // X d���m�n� y'nin sol �ocu�u olarak ata
    y->left = x->right; // X'in sa� �ocu�unu y'nin sol �ocu�u yap
    if (x->right) x->right->parent = y; // E�er X'in sa� �ocu�u varsa, ebeveynini y olarak ata
    x->parent = y->parent; // X'in ebeveynini y'nin ebeveyni olarak ata
    if (!y->parent) root = x; // E�er y'nin ebeveyni yoksa, root'u x olarak ata
    else if (y == y->parent->left) y->parent->left = x; // y sol �ocuksa, ebeveynin sol �ocu�u x olsun
    else y->parent->right = x; // y sa� �ocuksa, ebeveynin sa� �ocu�u x olsun
    x->right = y; // y'yi x'in sa� �ocu�u yap
    y->parent = x; // y'nin ebeveynini x olarak ata
}

// Red-Black Tree'ye d���m eklerken d�zeltme i�lemi
void rbInsertFixup(RBNode* node) {
    RBNode* uncle; // Amca d���m�n� tan�mla
    while (node->parent && node->parent->color == RED) { // E�er ebeveyn k�rm�z�ysa
        if (node->parent == node->parent->parent->left) { // E�er ebeveyn soldaysa
            uncle = node->parent->parent->right; // Amcay� sa� �ocuk olarak ata
            if (uncle && uncle->color == RED) { // E�er amca k�rm�z�ysa
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                uncle->color = BLACK; // Amcan�n rengini siyah yap
                node->parent->parent->color = RED; // B�y�k ebeveynin rengini k�rm�z� yap
                node = node->parent->parent; // Node'u b�y�k ebeveyne ta��
            } else { // Amca siyahsa
                if (node == node->parent->right) { // E�er node sa� �ocuksa
                    node = node->parent; // Node'u ebeveyne ta��
                    leftRotate(node); // Ebeveyn �zerinde sola d�nd�rme yap
                }
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                node->parent->parent->color = RED; // B�y�k ebeveynin rengini k�rm�z� yap
                rightRotate(node->parent->parent); // B�y�k ebeveyn �zerinde sa�a d�nd�rme yap
            }
        } else { // E�er ebeveyn sa� �ocuksa
            uncle = node->parent->parent->left; // Amcay� sol �ocuk olarak ata
            if (uncle && uncle->color == RED) { // E�er amca k�rm�z�ysa
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                uncle->color = BLACK; // Amcan�n rengini siyah yap
                node->parent->parent->color = RED; // B�y�k ebeveynin rengini k�rm�z� yap
                node = node->parent->parent; // Node'u b�y�k ebeveyne ta��
            } else { // Amca siyahsa
                if (node == node->parent->left) { // E�er node sol �ocuksa
                    node = node->parent; // Node'u ebeveynine ta��
                    rightRotate(node); // Ebeveyn �zerinde sa�a d�nd�rme yap
                }
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                node->parent->parent->color = RED; // B�y�k ebeveynin rengini k�rm�z� yap
                leftRotate(node->parent->parent); // B�y�k ebeveyn �zerinde sola d�nd�rme yap
            }
        }
    }
    root->color = BLACK; // K�k d���m�n rengini siyah yap
}

// Kullan�c�y� Red-Black Tree'ye ekler
void rbInsert(User* user) {
    RBNode* node = createRBNode(user); // Yeni d���m olu�tur
    RBNode* y = NULL; // Ge�ici bir ebeveyn d���m�
    RBNode* x = root; // K�k d���mden ba�la
    while (x != NULL) { // K�kten ba�layarak do�ru yeri bul
        y = x; // Ebeveyni kaydet
        if (node->id < x->id) // E�er ID daha k���kse
            x = x->left; // Sol �ocu�a git
        else
            x = x->right; // Sa� �ocu�a git
    }
    node->parent = y; // Ebeveyni ata
    if (y == NULL) // E�er ebeveyn yoksa (root)
        root = node; // K�k olarak ata
    else if (node->id < y->id) // E�er ID daha k���kse
        y->left = node; // Sol �ocu�a ata
    else
        y->right = node; // Sa� �ocu�a ata
    rbInsertFixup(node); // Red-Black Tree'yi d�zelt
}

User* rbSearch(int id) {
    RBNode* current = root; // K�k d���mden ba�la
    while (current != NULL) {
        if (id == current->id) // ID'yi bulana kadar dola�
            return current->user;
        else if (id < current->id) // E�er ID daha k���kse
            current = current->left;  // Sol �ocu�a git
        else
            current = current->right; // Sa� �ocu�a git
    }
    return NULL; // Bulunan d���m� d�nd�r (veya NULL)
}

// Ana Fonksiyon
int main() {
    readDataset("veriseti.txt");

    printf("\n- Kullanici 101 i�in 1 mesafe uzakliktaki arkadaslar:\n");
    findFriendsAtDistance(102, 1);

    printf("\n- 101 ve 102 ortak arkadaslari:\n");
    findCommonFriends(101, 103);

    printf("\n- Topluluklar:\n");
    detectCommunities();

    printf("\n- 104 numarali kullanicinin etki alani:\n");
    calculateInfluence(105);

    printf("\n- Red-Black Tree'den 103 aramasi:\n");
    User* foundUser = rbSearch(104);
    if (foundUser) printf("Kullanici bulundu: %d\n", foundUser->id);
    else printf("Kullanici bulunamadi.\n");

    return 0;
}

