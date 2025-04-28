#include <stdio.h> // Giriþ/çýkýþ iþlemleri için standart kütüphane
#include <stdlib.h> // Bellek yönetimi ve genel amaçlý fonksiyonlar için standart kütüphane
#include <string.h> // String iþlemleri için standart kütüphane

#define MAX_USERS 1000 // Maksimum kullanýcý sayýsý

// Kullanýcý Yapýsý
typedef struct User {
    int id; // Kullanýcýnýn ID'si
    int friendCount; // Arkadaþ sayýsý
    struct User** friends; // Kullanýcýnýn arkadaþlarýnýn listesi
} User;

// Red-Black Tree Node Yapýsý
typedef enum { RED, BLACK } Color; // Düðüm renklerini temsil eden enum

typedef struct RBNode {
    int id; // Düðümün temsil ettiði kullanýcýnýn ID'si
    User* user; // Düðümün iþaret ettiði kullanýcý
    Color color; // Düðümün rengi (kýrmýzý veya siyah)
    struct RBNode* left; // Sol çocuk düðüm
    struct RBNode* right; // Sað çocuk düðüm
    struct RBNode* parent; // Üst (ebeveyn) düðüm
} RBNode;

// Global Deðiþkenler
User* users[MAX_USERS]; // Kullanýcýlarý tutan dizi
int userCount = 0; // Kullanýcý sayacý
RBNode* root = NULL; // Red-Black Tree'nin kökü

// Fonksiyon Ýmza Bildirimleri
User* createUser(int id); // Yeni kullanýcý oluþturur
void addFriendship(int id1, int id2); // Ýki kullanýcý arasýnda arkadaþlýk oluþturur
User* getUser(int id); // ID ile kullanýcý arar
void readDataset(const char* filename); // Veriseti dosyasýný okur
void dfs(User* user, int currentDepth, int targetDepth, int* visited, int* found, int* foundCount); // DFS algoritmasý
void findFriendsAtDistance(int id, int distance); // Belirli mesafedeki arkadaþlarý bulur
void findCommonFriends(int id1, int id2); // Ýki kullanýcý arasýndaki ortak arkadaþlarý bulur
void exploreCommunity(User* user, int* visited, int* community, int* communitySize); // Topluluðu keþfeder
void detectCommunities(); // Topluluklarý tespit eder
void dfsInfluence(User* user, int* visited, int* count); // Etki alaný için DFS algoritmasý
void calculateInfluence(int id); // Kullanýcýnýn etki alanýný hesaplar
void rbInsert(User* user); // Red-Black Tree'ye kullanýcý ekler
void rbInsertFixup(RBNode* node); // Red-Black Tree'yi düzeltir
void leftRotate(RBNode* x); // Red-Black Tree'de sola döndürme
void rightRotate(RBNode* y); // Red-Black Tree'de saða döndürme
RBNode* createRBNode(User* user); // Red-Black Tree düðümü oluþturur
User* rbSearch(int id); // Red-Black Tree içinde kullanýcý arar

// Kullanýcý oluþtur
User* createUser(int id) {
    User* user = (User*)malloc(sizeof(User)); // Kullanýcý için hafýza ayýr
    user->id = id; // ID'yi ata
    user->friendCount = 0; // Baþlangýçta arkadaþ sayýsý 0
    user->friends = (User**)malloc(sizeof(User*) * MAX_USERS); // Arkadaþlar dizisi için hafýza ayýr
    return user; // Kullanýcýyý döndür
}

// ID ile kullanýcýyý getir
User* getUser(int id) {
    int i = 0; // Sayaç deðiþkeni
    for (i = 0; i < userCount; i++) { // Tüm kullanýcýlar içinde dolaþ
        if (users[i]->id == id) { // ID eþleþirse
            return users[i]; // Kullanýcýyý döndür
        }
    }
    return NULL; // Bulunamazsa NULL döndür
}

// Ýki kullanýcý arasýnda arkadaþlýk kur
void addFriendship(int id1, int id2) {
    User* user1 = getUser(id1); // Ýlk kullanýcýyý bul
    User* user2 = getUser(id2); // Ýkinci kullanýcýyý bul
    if (user1 && user2) { // Ýkisi de varsa
        user1->friends[user1->friendCount++] = user2; // user2'yi user1'in arkadaþ listesine ekle
        user2->friends[user2->friendCount++] = user1; // user1'i user2'nin arkadaþ listesine ekle
    }
}

// Veriseti dosyasýný oku
void readDataset(const char* filename) {
    FILE* file = fopen(filename, "r"); // Dosyayý oku modunda aç
    if (!file) { // Dosya açýlamadýysa
        printf("Dosya açilamadi.\n"); // Hata mesajý ver
        return; // Fonksiyondan çýk
    }
    
    char line[100]; // Satýr okuma buffer'ý
    while (fgets(line, sizeof(line), file)) { // Satýr satýr oku
        char type[10]; // Satýrýn tipini sakla (USER ya da FRIEND)
        int id1, id2; // Kullanýcý ID'leri
        sscanf(line, "%s", type); // Satýr tipini oku
        if (strcmp(type, "USER") == 0) { // Satýr tipi USER ise
            sscanf(line, "%*s %d", &id1); // ID'yi al
            User* user = createUser(id1); // Yeni kullanýcý oluþtur
            users[userCount++] = user; // Kullanýcýyý listeye ekle
            rbInsert(user); // Red-Black Tree'ye kullanýcýyý ekle
        } else if (strcmp(type, "FRIEND") == 0) { // Satýr tipi FRIEND ise
            sscanf(line, "%*s %d %d", &id1, &id2); // Ýki ID'yi oku
            addFriendship(id1, id2); // Ýki kullanýcý arasýnda arkadaþlýk oluþtur
        }
    }
    fclose(file); // Dosyayý kapat
}

// DFS algoritmasý (Depth First Search)
void dfs(User* user, int currentDepth, int targetDepth, int* visited, int* found, int* foundCount) {
    int i = 0; // Sayaç deðiþkeni
    if (!user || visited[user->id]) return; // Kullanýcý yoksa veya ziyaret edildiyse çýk
    visited[user->id] = 1; // Kullanýcýyý ziyaret edildi olarak iþaretle
    
    if (currentDepth == targetDepth) { // Hedef derinliðe ulaþýldýysa
        found[(*foundCount)++] = user->id; // Bulunan kullanýcý listesine ekle
        return; // Daha ileri gitme
    }
    
    for (i = 0; i < user->friendCount; i++) { // Arkadaþlar arasýnda dolaþ
        dfs(user->friends[i], currentDepth + 1, targetDepth, visited, found, foundCount); // Derinliði artýrarak DFS uygula
    }
}

// Belirli mesafedeki arkadaþlarý bulur
void findFriendsAtDistance(int id, int distance) {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int found[MAX_USERS]; // Bulunan kullanýcýlar
    int foundCount = 0; // Bulunan kullanýcý sayýsý
    int i = 0; // Sayaç deðiþkeni
    
    User* user = getUser(id); // Kullanýcýyý bul
    if (!user) { // Kullanýcý bulunamadýysa
        printf("Kullanici bulunamadi.\n"); // Hata mesajý ver
        return; // Fonksiyondan çýk
    }
    
    dfs(user, 0, distance, visited, found, &foundCount); // DFS ile kullanýcýlarý ara
    
    printf("%d mesafedeki arkadaslar: ", distance); // Sonuçlarý yazdýr
    for (i = 0; i < foundCount; i++) { // Bulunanlarý yazdýr
        printf("%d ", found[i]);
    }
    printf("\n");
}

// Ortak arkadaþlarý bulur
void findCommonFriends(int id1, int id2) {
    User* user1 = getUser(id1); // Ýlk kullanýcýyý bul
    User* user2 = getUser(id2); // Ýkinci kullanýcýyý bul
    int i = 0, j = 0; // Sayaç deðiþkenleri
    
    if (!user1 || !user2) { // Kullanýcý(lar) bulunamadýysa
        printf("Kullanici(lar) bulunamadi.\n"); // Hata mesajý ver
        return; // Fonksiyondan çýk
    }
    
    printf("%d ve %d ortak arkadaslari: ", id1, id2); // Baþlýk yaz
    for (i = 0; i < user1->friendCount; i++) { // user1'in arkadaþlarýnda dolaþ
        for (j = 0; j < user2->friendCount; j++) { // user2'nin arkadaþlarýnda dolaþ
            if (user1->friends[i]->id == user2->friends[j]->id) { // Ortak arkadaþ bulunduysa
                printf("%d ", user1->friends[i]->id); // Ortak arkadaþý yazdýr
            }
        }
    }
    printf("\n");
}

void exploreCommunity(User* user, int* visited, int* community, int* communitySize) {
    int i = 0; // Sayaç deðiþkeni
    if (visited[user->id]) return; // Kullanýcý daha önce ziyaret edildiyse çýk
    visited[user->id] = 1; // Kullanýcýyý ziyaret edildi olarak iþaretle
    community[(*communitySize)++] = user->id; // Kullanýcýyý topluluða ekle
    
    for (i = 0; i < user->friendCount; i++) { // Kullanýcýnýn arkadaþlarýnda dolaþ
        exploreCommunity(user->friends[i], visited, community, communitySize); // Arkadaþlarýný da ziyaret et
    }
}

// Topluluklarý tespit eder
void detectCommunities() {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int i = 0; // Sayaç deðiþkeni
    int community[MAX_USERS]; // Topluluðu tutacak dizi
    int communitySize = 0; // Topluluðun büyüklüðü
    
    printf("Topluluklar:\n"); // Baþlýk yazdýr
    
    for (i = 0; i < userCount; i++) { // Tüm kullanýcýlar üzerinde döngü baþlat
        if (!visited[users[i]->id]) { // Kullanýcý henüz ziyaret edilmediyse
            communitySize = 0; // Yeni topluluk oluþturulacak
            exploreCommunity(users[i], visited, community, &communitySize); // Topluluðu keþfet
            
            int j = 0; // Sayaç deðiþkeni
            printf("[ "); // Topluluklarý yazdýrmaya baþla
            for (j = 0; j < communitySize; j++) { // Topluluðu yazdýr
                printf("%d ", community[j]);
            }
            printf("]\n"); // Topluluðun bitiþi
        }
    }
}

// Etki alaný için DFS algoritmasý
void dfsInfluence(User* user, int* visited, int* count) {
    int i = 0; // Sayaç deðiþkeni
    if (visited[user->id]) return; // Eðer kullanýcýyý zaten ziyaret ettiysek çýk
    visited[user->id] = 1; // Kullanýcýyý ziyaret edildi olarak iþaretle
    (*count)++; // Etki alaný sayýsýný artýr
    
    for (i = 0; i < user->friendCount; i++) { // Kullanýcýnýn arkadaþlarýnda dolaþ
        dfsInfluence(user->friends[i], visited, count); // Derinlemesine gez
    }
}

// Kullanýcýnýn etki alanýný hesaplar
void calculateInfluence(int id) {
    int visited[MAX_USERS] = {0}; // Ziyaret edilenler dizisi
    int count = 0; // Etki alaný sayacý
    
    User* user = getUser(id); // Kullanýcýyý bul
    if (!user) { // Kullanýcý bulunamazsa
        printf("Kullanici bulunamadi.\n"); // Hata mesajý ver
        return; // Fonksiyondan çýk
    }
    
    dfsInfluence(user, visited, &count); // Etki alaný hesaplamak için DFS uygula
    printf("%d numarali kullanicinin etki alani: %d\n", id, count - 1); // Sonucu yazdýr (kendisi dahil edilmemeli)
}

// Red-Black Tree iþlemleri

// Yeni Red-Black Tree düðümü oluþturur
RBNode* createRBNode(User* user) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode)); // Düðüm için hafýza ayýr
    node->id = user->id; // Kullanýcý ID'sini düðüme ata
    node->user = user; // Kullanýcýyý düðüme ata
    node->color = RED; // Düðüm rengini kýrmýzý olarak ata
    node->left = node->right = node->parent = NULL; // Düðümün çocuklarýný ve ebeveynini NULL yap
    return node; // Yeni düðümü döndür
}

// Red-Black Tree'de sola döndürme
void leftRotate(RBNode* x) {
    RBNode* y = x->right; // Y düðümünü x'in sað çocuðu olarak ata
    x->right = y->left; // Y'nin sol çocuðunu x'in sað çocuðu yap
    if (y->left) y->left->parent = x; // Eðer Y'nin sol çocuðu varsa, ebeveynini x olarak ata
    y->parent = x->parent; // Y'nin ebeveynini x'in ebeveyni olarak ata
    if (!x->parent) root = y; // Eðer x'in ebeveyni yoksa, root'u y olarak ata
    else if (x == x->parent->left) x->parent->left = y; // x sol çocuksa, ebeveynin sol çocuðu y olsun
    else x->parent->right = y; // x sað çocuksa, ebeveynin sað çocuðu y olsun
    y->left = x; // x'i y'nin sol çocuðu yap
    x->parent = y; // x'in ebeveynini y olarak ata
}

// Red-Black Tree'de saða döndürme
void rightRotate(RBNode* y) {
    RBNode* x = y->left; // X düðümünü y'nin sol çocuðu olarak ata
    y->left = x->right; // X'in sað çocuðunu y'nin sol çocuðu yap
    if (x->right) x->right->parent = y; // Eðer X'in sað çocuðu varsa, ebeveynini y olarak ata
    x->parent = y->parent; // X'in ebeveynini y'nin ebeveyni olarak ata
    if (!y->parent) root = x; // Eðer y'nin ebeveyni yoksa, root'u x olarak ata
    else if (y == y->parent->left) y->parent->left = x; // y sol çocuksa, ebeveynin sol çocuðu x olsun
    else y->parent->right = x; // y sað çocuksa, ebeveynin sað çocuðu x olsun
    x->right = y; // y'yi x'in sað çocuðu yap
    y->parent = x; // y'nin ebeveynini x olarak ata
}

// Red-Black Tree'ye düðüm eklerken düzeltme iþlemi
void rbInsertFixup(RBNode* node) {
    RBNode* uncle; // Amca düðümünü tanýmla
    while (node->parent && node->parent->color == RED) { // Eðer ebeveyn kýrmýzýysa
        if (node->parent == node->parent->parent->left) { // Eðer ebeveyn soldaysa
            uncle = node->parent->parent->right; // Amcayý sað çocuk olarak ata
            if (uncle && uncle->color == RED) { // Eðer amca kýrmýzýysa
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                uncle->color = BLACK; // Amcanýn rengini siyah yap
                node->parent->parent->color = RED; // Büyük ebeveynin rengini kýrmýzý yap
                node = node->parent->parent; // Node'u büyük ebeveyne taþý
            } else { // Amca siyahsa
                if (node == node->parent->right) { // Eðer node sað çocuksa
                    node = node->parent; // Node'u ebeveyne taþý
                    leftRotate(node); // Ebeveyn üzerinde sola döndürme yap
                }
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                node->parent->parent->color = RED; // Büyük ebeveynin rengini kýrmýzý yap
                rightRotate(node->parent->parent); // Büyük ebeveyn üzerinde saða döndürme yap
            }
        } else { // Eðer ebeveyn sað çocuksa
            uncle = node->parent->parent->left; // Amcayý sol çocuk olarak ata
            if (uncle && uncle->color == RED) { // Eðer amca kýrmýzýysa
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                uncle->color = BLACK; // Amcanýn rengini siyah yap
                node->parent->parent->color = RED; // Büyük ebeveynin rengini kýrmýzý yap
                node = node->parent->parent; // Node'u büyük ebeveyne taþý
            } else { // Amca siyahsa
                if (node == node->parent->left) { // Eðer node sol çocuksa
                    node = node->parent; // Node'u ebeveynine taþý
                    rightRotate(node); // Ebeveyn üzerinde saða döndürme yap
                }
                node->parent->color = BLACK; // Ebeveynin rengini siyah yap
                node->parent->parent->color = RED; // Büyük ebeveynin rengini kýrmýzý yap
                leftRotate(node->parent->parent); // Büyük ebeveyn üzerinde sola döndürme yap
            }
        }
    }
    root->color = BLACK; // Kök düðümün rengini siyah yap
}

// Kullanýcýyý Red-Black Tree'ye ekler
void rbInsert(User* user) {
    RBNode* node = createRBNode(user); // Yeni düðüm oluþtur
    RBNode* y = NULL; // Geçici bir ebeveyn düðümü
    RBNode* x = root; // Kök düðümden baþla
    while (x != NULL) { // Kökten baþlayarak doðru yeri bul
        y = x; // Ebeveyni kaydet
        if (node->id < x->id) // Eðer ID daha küçükse
            x = x->left; // Sol çocuða git
        else
            x = x->right; // Sað çocuða git
    }
    node->parent = y; // Ebeveyni ata
    if (y == NULL) // Eðer ebeveyn yoksa (root)
        root = node; // Kök olarak ata
    else if (node->id < y->id) // Eðer ID daha küçükse
        y->left = node; // Sol çocuða ata
    else
        y->right = node; // Sað çocuða ata
    rbInsertFixup(node); // Red-Black Tree'yi düzelt
}

User* rbSearch(int id) {
    RBNode* current = root; // Kök düðümden baþla
    while (current != NULL) {
        if (id == current->id) // ID'yi bulana kadar dolaþ
            return current->user;
        else if (id < current->id) // Eðer ID daha küçükse
            current = current->left;  // Sol çocuða git
        else
            current = current->right; // Sað çocuða git
    }
    return NULL; // Bulunan düðümü döndür (veya NULL)
}

// Ana Fonksiyon
int main() {
    readDataset("veriseti.txt");

    printf("\n- Kullanici 101 için 1 mesafe uzakliktaki arkadaslar:\n");
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

